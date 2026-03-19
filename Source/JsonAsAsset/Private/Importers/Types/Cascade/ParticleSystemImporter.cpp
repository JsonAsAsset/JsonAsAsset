/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Importers/Types/Cascade/ParticleSystemImporter.h"

#include "Settings/Runtime.h"

/* Particle System Includes */
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleModuleRequired.h"
#include "Particles/ParticleSystem.h"

#include "Particles/Spawn/ParticleModuleSpawn.h"
#include "Particles/TypeData/ParticleModuleTypeDataBase.h"
#include "Utilities/JsonUtilities.h"

UObject* IParticleSystemImporter::CreateAsset(UObject* CreatedAsset) {
	return IImporter::CreateAsset(NewObject<UParticleSystem>(GetPackage(), UParticleSystem::StaticClass(), *GetAssetName(), RF_Public | RF_Standalone));
}

bool IParticleSystemImporter::Import() {
	const auto ParticleSystem = Create<UParticleSystem>();

	/* Create already existing distributions */
	CreateDistributions();
	
	/* Ensure any default emitters are cleared */
	WipeEmitters();

	/* Emitters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	CreateEmitters(GetAssetDataAsValue().GetArray("Emitters"));

	GetObjectSerializer()->DeserializeObjectProperties(
		RemovePropertiesShared(GetAssetData(), {
			"RequiredModule",
			"Modules",
			"TypeDataModule",
			"SpawnModule",
			"Emitters"
		}
	), ParticleSystem);

	/* Handle edit changes, and add it to the content browser */
	return OnAssetCreation(ParticleSystem);
}

void IParticleSystemImporter::CreateDistributions() {
	const auto ParticleSystem = GetTypedAsset<UParticleSystem>();
	
	for (FUObjectExport& Export : AssetContainer.GetExportsWithPropertyNameStartingWith("Type", "Distribution")) {
		/* Create Distribution */
		UObject* Distribution = NewObject<UDistribution>(GetAsset(), Export.GetClass(), Export.GetName());
		if (!Distribution) break;

		if (Export.Has("Properties")) {
			GetObjectSerializer()->DeserializeObjectProperties(Export.GetProperties(), Distribution);
		}

		Export.SetPosition(-1);
		Export.SetParent(ParticleSystem);
		Export.SetObject(Distribution);

		GetPropertySerializer()->ExportsContainer.Exports.Add(Export);
	}
}

void IParticleSystemImporter::WipeEmitters() const {
	const auto ParticleSystem = GetTypedAsset<UParticleSystem>();
	
	if (ParticleSystem->Emitters.Num() > 0) {
		/* Destroy any existing emitters */
		for (UParticleEmitter* Emitter : ParticleSystem->Emitters) {
			if (Emitter) {
				Emitter->ConditionalBeginDestroy();
			}
		}

		/* Clear the emitters array */
		ParticleSystem->Emitters.Empty();
	}
}

void IParticleSystemImporter::CreateEmitters(const TArray<FUObjectJsonValueExport>& Exports) {
	const auto ParticleSystem = GetTypedAsset<UParticleSystem>();

	AssetContainer.ExportsLoop(Exports, [this, ParticleSystem](FUObjectExport& DirectExport) {
		ParticleSystem->PreEditChange(nullptr);

		CreateEmitter(DirectExport.GetClass(), DirectExport.GetName(), DirectExport);
	});
}

UParticleEmitter* IParticleSystemImporter::CreateEmitter(const UClass* Class, const FName Name, const FUObjectExport& Export) {
	const auto ParticleSystem = GetTypedAsset<UParticleSystem>();
	
	UParticleEmitter* Emitter = NewObject<UParticleEmitter>(ParticleSystem, Class, Name, RF_Transactional);

	/* Add to particle system */
	ParticleSystem->Emitters.Add(Emitter);

	/* Create LODLevels ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	AssetContainer.ExportsLoop(
		Export.GetPropertiesAsValue().GetArray("LODLevels"),
		
		[this, Emitter](const FUObjectExport& DirectExport) {
			CreateLODLevel(DirectExport, Emitter);
		}
	);
	
	Emitter->EmitterEditorColor = FColor::MakeRandomColor();
	Emitter->EmitterEditorColor.A = 255;

	Emitter->UpdateModuleLists();
	Emitter->PostEditChange();

	ParticleSystem->PostEditChange();
	ParticleSystem->SetupSoloing();

	GetObjectSerializer()->DeserializeObjectProperties(
		RemovePropertiesShared(Export.GetProperties(), {
				"RequiredModule",
				"Modules",
				"TypeDataModule",
				"SpawnModule",
				"Emitters",
				"LODLevels"
			}
	), Emitter);
	
	/* Initialize epic detail mode to enabled if it's an older version of the engine */
	if (!GJsonAsAssetRuntime.IsUE5()) {
#if ENGINE_UE5
		if (Emitter->DetailModeBitmask & 1 << EParticleDetailMode::PDM_High) {
			Emitter->DetailModeBitmask |= 1 << EParticleDetailMode::PDM_Epic;
		}
#endif
	}
	
	return Emitter;
}

void IParticleSystemImporter::CreateLODLevel(const FUObjectExport& Export, UParticleEmitter* Emitter) {
	const auto ParticleSystem = GetTypedAsset<UParticleSystem>();

	const FUObjectJsonValueExport JsonValue = Export.GetPropertiesAsValue();
	
	/* Find the LOD Level if it already exists */
	UParticleLODLevel* LODLevel = Emitter->GetLODLevel(JsonValue.GetInteger("Level", 0));

	/* Create a LOD Level if it doesn't exist */
	if (LODLevel == nullptr) {
		LODLevel = NewObject<UParticleLODLevel>(Emitter, UParticleLODLevel::StaticClass(), Export.GetName());
		check(LODLevel);

		LODLevel->ConvertedModules = true;
		Emitter->LODLevels.Add(LODLevel);
	}

	GetObjectSerializer()->DeserializeObjectProperties(
		RemovePropertiesShared(JsonValue.JsonObject, {
				"RequiredModule",
				"Modules",
				"TypeDataModule",
				"SpawnModule"
			}
	), LODLevel);

	/* Required Module */
	if (JsonValue.Has("RequiredModule")) {
		const FUObjectExport ModulePath = AssetContainer.GetExportByObjectPath(JsonValue.GetObject("RequiredModule"));

		/* Create the module */
		UParticleModuleRequired* RequiredModule = NewObject<UParticleModuleRequired>(ParticleSystem, UParticleModuleRequired::StaticClass(), ModulePath.GetName(), RF_Transactional);
		LODLevel->RequiredModule = RequiredModule;
		RequiredModule->ModuleEditorColor = FColor::MakeRandomColor();
		
		DeserializeModule(ModulePath.GetProperties(), LODLevel->RequiredModule);
	}

	/* Spawn Module */
	if (JsonValue.Has("SpawnModule")) {
		const FUObjectExport ModulePath = AssetContainer.GetExportByObjectPath(JsonValue.GetObject("SpawnModule"));

		/* Create the module */
		UParticleModuleSpawn* SpawnModule = NewObject<UParticleModuleSpawn>(ParticleSystem, UParticleModuleSpawn::StaticClass(), ModulePath.GetName(), RF_Transactional);
		LODLevel->SpawnModule = SpawnModule;
		SpawnModule->BurstList.Empty();
		
		DeserializeModule(ModulePath.GetProperties(), LODLevel->SpawnModule);
	}

	/* Type Data Module */
	if (JsonValue.Has("TypeDataModule")) {
		FUObjectExport ModulePath = AssetContainer.GetExportByObjectPath(JsonValue.GetObject("TypeDataModule"));

		UParticleModuleTypeDataBase* TypeDataModule = NewObject<UParticleModuleTypeDataBase>(ParticleSystem, ModulePath.GetClass(), ModulePath.GetName(), RF_Transactional);
		check(TypeDataModule);
		
		LODLevel->TypeDataModule = TypeDataModule;
		DeserializeModule(ModulePath.GetProperties(), TypeDataModule);
	}

	for (const FUObjectJsonValueExport& ModulePath : JsonValue.GetArray("Modules")) {
		FUObjectExport ModuleExport = AssetContainer.GetExportByObjectPath(ModulePath);
		
		UParticleModule* Module = NewObject<UParticleModule>(ParticleSystem, ModuleExport.GetClass(), ModuleExport.GetName(), RF_Transactional);
		check(Module);
		
		Module->ModuleEditorColor = FColor::MakeRandomColor();
		Module->SetTransactionFlag();
		
		LODLevel->Modules.Add(Module);

		ParticleSystem->PostEditChange();
		if (!ParticleSystem->MarkPackageDirty()) return;

		DeserializeModule(ModuleExport.GetProperties(), Module);
	}

	Emitter->PostEditChange();
}

void IParticleSystemImporter::DeserializeModule(const TSharedPtr<FJsonObject>& ModuleProperties, UParticleModule* Module) const {
	GetObjectSerializer()->Parent = Module;

	GetObjectSerializer()->DeserializeObjectProperties(
		RemovePropertiesShared(ModuleProperties, {
			"RequiredModule",
			"Modules",
			"TypeDataModule",
			"SpawnModule",
			"Emitters"
		}
	), Module);

	Module->PostLoad();
}