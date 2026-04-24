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

	/* Ensure any default emitters are cleared */
	WipeEmitters();

	/* Emitters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	GetObjectSerializer()->bUseExperimentalSpawning = true;
	GetObjectSerializer()->DeserializeObjectProperties(GetAssetData(), ParticleSystem);

	/* Handle edit changes, and add it to the content browser */
	return OnAssetCreation(ParticleSystem);
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

	GetContainer()->ExportsLoop(Exports, [this, ParticleSystem](FUObjectExport* DirectExport) {
		ParticleSystem->PreEditChange(nullptr);

		CreateEmitter(DirectExport->GetClass(), DirectExport->GetName(), DirectExport);
	});
}

UParticleEmitter* IParticleSystemImporter::CreateEmitter(const UClass* Class, const FName Name, FUObjectExport* Export) {
	const auto ParticleSystem = GetTypedAsset<UParticleSystem>();
	
	UParticleEmitter* Emitter = NewObject<UParticleEmitter>(ParticleSystem, Class, Name, RF_Transactional);

	/* Add to particle system */
	ParticleSystem->Emitters.Add(Emitter);
	Export->Object = Emitter;
	
	GetObjectSerializer()->WhitelistedTreeSegments = Export->GetOuterTreeSegments();
	DeserializeExports(ParticleSystem, true);
	
	/* Create LODLevels ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	GetContainer()->ExportsLoop(
		Export->GetPropertiesAsValue().GetArray("LODLevels"),
		
		[this, Emitter](const FUObjectExport* DirectExport) {
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
		RemovePropertiesShared(Export->GetProperties(), {
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
#if ENGINE_UE5 && ENGINE_MINOR_VERSION >= 2
		if (Emitter->DetailModeBitmask & 1 << EParticleDetailMode::PDM_High) {
			Emitter->DetailModeBitmask |= 1 << EParticleDetailMode::PDM_Epic;
		}
#endif
	}
	
	return Emitter;
}

void IParticleSystemImporter::CreateLODLevel(const FUObjectExport* Export, UParticleEmitter* Emitter) const {
	const auto ParticleSystem = GetTypedAsset<UParticleSystem>();

	const FUObjectJsonValueExport JsonValue = Export->GetPropertiesAsValue();
	
	/* Find the LOD Level if it already exists */
	UParticleLODLevel* LODLevel = Emitter->GetLODLevel(JsonValue.GetInteger("Level", 0));

	/* Create a LOD Level if it doesn't exist */
	if (LODLevel == nullptr) {
		LODLevel = NewObject<UParticleLODLevel>(Emitter, UParticleLODLevel::StaticClass(), Export->GetName());
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
		FUObjectExport* ModulePath = GetContainer()->GetExportByObjectPath(JsonValue.GetObject("RequiredModule"));

		/* Create the module */
		UParticleModuleRequired* RequiredModule = NewObject<UParticleModuleRequired>(ParticleSystem, UParticleModuleRequired::StaticClass(), ModulePath->GetName(), RF_Transactional);
		LODLevel->RequiredModule = RequiredModule;
		RequiredModule->ModuleEditorColor = FColor::MakeRandomColor();
		
		DeserializeModule(ModulePath, LODLevel->RequiredModule);
	}

	/* Spawn Module */
	if (JsonValue.Has("SpawnModule")) {
		FUObjectExport* ModulePath = GetContainer()->GetExportByObjectPath(JsonValue.GetObject("SpawnModule"));

		/* Create the module */
		UParticleModuleSpawn* SpawnModule = NewObject<UParticleModuleSpawn>(ParticleSystem, UParticleModuleSpawn::StaticClass(), ModulePath->GetName(), RF_Transactional);
		LODLevel->SpawnModule = SpawnModule;
		SpawnModule->BurstList.Empty();
		
		DeserializeModule(ModulePath, LODLevel->SpawnModule);
	}

	/* Type Data Module */
	if (JsonValue.Has("TypeDataModule")) {
		FUObjectExport* ModulePath = GetContainer()->GetExportByObjectPath(JsonValue.GetObject("TypeDataModule"));

		UParticleModuleTypeDataBase* TypeDataModule = NewObject<UParticleModuleTypeDataBase>(ParticleSystem, ModulePath->GetClass(), ModulePath->GetName(), RF_Transactional);
		check(TypeDataModule);
		
		LODLevel->TypeDataModule = TypeDataModule;
		DeserializeModule(ModulePath, TypeDataModule);
	}

	for (const FUObjectJsonValueExport& ModulePath : JsonValue.GetArray("Modules")) {
		FUObjectExport* ModuleExport = GetContainer()->GetExportByObjectPath(ModulePath);
		
		UParticleModule* Module = NewObject<UParticleModule>(ParticleSystem, ModuleExport->GetClass(), ModuleExport->GetName(), RF_Transactional);
		check(Module);
		
		Module->ModuleEditorColor = FColor::MakeRandomColor();
		Module->SetTransactionFlag();
		
		LODLevel->Modules.Add(Module);

		ParticleSystem->PostEditChange();
		if (!ParticleSystem->MarkPackageDirty()) return;

		DeserializeModule(ModuleExport, Module);
	}

	Emitter->PostEditChange();
}

void IParticleSystemImporter::DeserializeModule(FUObjectExport* Export, UParticleModule* Module) const {
	GetObjectSerializer()->Parent = Module;

	Export->Object = Module;
	
	GetObjectSerializer()->WhitelistedTreeSegments = Export->GetOuterTreeSegments();
	GetObjectSerializer()->DeserializeExports(GetContainer(), true);

	GetObjectSerializer()->DeserializeObjectProperties(
		RemovePropertiesShared(Export->GetProperties(), {
			"RequiredModule",
			"Modules",
			"TypeDataModule",
			"SpawnModule",
			"Emitters"
		}
	), Module);

	Module->PostLoad();
}
