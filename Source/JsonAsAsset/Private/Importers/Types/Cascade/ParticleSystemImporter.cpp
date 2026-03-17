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

	GetObjectSerializer()->DeserializeObjectProperties(RemovePropertiesShared(GetAssetData(),
	{
		"RequiredModule",
		"Modules",
		"TypeDataModule",
		"SpawnModule",
		"Emitters"
	}), ParticleSystem);

	/* Handle edit changes, and add it to the content browser */
	return OnAssetCreation(ParticleSystem);
}

void IParticleSystemImporter::CreateDistributions() {
	const auto ParticleSystem = GetTypedAsset<UParticleSystem>();
	
	for (FUObjectExport Export : AssetContainer.GetExportsWithPropertyNameStartingWith("Type", "Distribution")) {
		/* Create Distribution */
		UObject* Distribution = NewObject<UDistribution>(GetAsset(), Export.GetClass());
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

	/* Setup LODLevels ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
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
	Emitter->SetFlags(RF_Transactional);

	ParticleSystem->PostEditChange();
	ParticleSystem->SetupSoloing();

	Emitter->UpdateModuleLists();
	Emitter->PostEditChange();
	Emitter->SetFlags(RF_Transactional);

	GetObjectSerializer()->DeserializeObjectProperties(RemovePropertiesShared(Export.GetProperties(),
	{
		"RequiredModule",
		"Modules",
		"TypeDataModule",
		"SpawnModule",
		"Emitters",
		"LODLevels"
	}), Emitter);
	
	return Emitter;
}

UParticleLODLevel* IParticleSystemImporter::CreateLODLevel(const FUObjectExport& Export, UParticleEmitter* Emitter) {
	const auto ParticleSystem = GetTypedAsset<UParticleSystem>();

	const FUObjectJsonValueExport JsonValue = Export.GetPropertiesAsValue();
	
	/* LOD Level by default is zero */
	const int Level = JsonValue.GetInteger("Level", 0);

	/* Find the LOD Level if it already exists */
	UParticleLODLevel* LODLevel = Emitter->GetLODLevel(Level);

	/* Create a LOD Level if it doesn't exist */
	if (LODLevel == nullptr) {
		LODLevel = NewObject<UParticleLODLevel>(Emitter);

		check(LODLevel);

		LODLevel->Level = Level;
		LODLevel->bEnabled = JsonValue.GetBool("bEnabled", true);
		
		LODLevel->ConvertedModules = true;
		LODLevel->PeakActiveParticles = 0;
		
		LODLevel->TypeDataModule = nullptr;
		Emitter->LODLevels.Add(LODLevel);
	}

	LODLevel->Modules.Empty();

	GetObjectSerializer()->DeserializeObjectProperties(RemovePropertiesShared(JsonValue.JsonObject, {
		"RequiredModule",
		"Modules",
		"TypeDataModule",
		"SpawnModule"
	}), LODLevel);

	/* Required Module */
	if (JsonValue.Has("RequiredModule")) {
		const FUObjectExport DirectExport = AssetContainer.GetExportByObjectPath(JsonValue.GetObject("RequiredModule"));

		/* Create the module */
		UParticleModuleRequired* RequiredModule = NewObject<UParticleModuleRequired>(ParticleSystem);
		LODLevel->RequiredModule = RequiredModule;
		RequiredModule->ModuleEditorColor = FColor::MakeRandomColor();
		
		DeserializeModule(DirectExport.GetProperties(), LODLevel->RequiredModule);
	}

	/* Spawn Module */
	if (JsonValue.Has("SpawnModule")) {
		const FUObjectExport DirectExport = AssetContainer.GetExportByObjectPath(JsonValue.GetObject("SpawnModule"));

		/* Create the module */
		UParticleModuleSpawn* SpawnModule = NewObject<UParticleModuleSpawn>(ParticleSystem);
		LODLevel->SpawnModule = SpawnModule;
		SpawnModule->BurstList.Empty();
		
		DeserializeModule(DirectExport.GetProperties(), LODLevel->SpawnModule);
	}

	/* TypeDataModule */
	if (JsonValue.Has("TypeDataModule")) {
		FUObjectExport DirectExport = AssetContainer.GetExportByObjectPath(JsonValue.GetObject("TypeDataModule"));

		UParticleModuleTypeDataBase* TypeDataModule = NewObject<UParticleModuleTypeDataBase>(ParticleSystem, DirectExport.GetClass());
		check(TypeDataModule);
		
		LODLevel->TypeDataModule = TypeDataModule;
		DeserializeModule(DirectExport.GetProperties(), TypeDataModule);
	}

	for (const FUObjectJsonValueExport& ModulesReference : JsonValue.GetArray("Modules")) {
		FUObjectExport DirectExport = AssetContainer.GetExportByObjectPath(ModulesReference);
		UParticleModule* Module = NewObject<UParticleModule>(ParticleSystem, DirectExport.GetClass(), FName(DirectExport.GetName()));
		
		check(Module);
		Module->ModuleEditorColor = FColor::MakeRandomColor();

		Module->SetTransactionFlag();
		
		LODLevel->Modules.Add(Module);

		ParticleSystem->PostEditChange();
		ParticleSystem->MarkPackageDirty();

		DeserializeModule(DirectExport.GetProperties(), Module);
	}

	Emitter->PostEditChange();
	Emitter->SetFlags(RF_Transactional);

	return nullptr;
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
