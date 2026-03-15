/* Copyright JAA Contributors 2024-2026 */

#include "Importers/Types/Cascade/ParticleSystemImporter.h"

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

	CreateDistributions();
	
	/* Ensure any default emitters are cleared */
	EmptyParticleSystem();

	/* Emitters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	CreateEmitters(GetAssetData()->GetArrayField(TEXT("Emitters")));

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
	
	TArray<FUObjectExport> DistributionObjects = AssetContainer.GetExportsWithPropertyNameStartingWith("Type", "Distribution");

	for (FUObjectExport JsonObjectValue : DistributionObjects) {
		/* Find Class */
		const UClass* Class = FindObject<UClass>(ANY_PACKAGE, *JsonObjectValue.GetType().ToString());

		/* Create Distribution */
		UObject* Distribution = NewObject<UDistribution>(GetAsset(), Class);

		if (JsonObjectValue.JsonObject->HasField(TEXT("Properties"))) {
			GetObjectSerializer()->DeserializeObjectProperties(JsonObjectValue.GetProperties(), Distribution);
		}

		GetPropertySerializer()->ExportsContainer.Exports.Add(FUObjectExport(FName(JsonObjectValue.GetName()), FName("Distribution"), FName(JsonObjectValue.GetOuter()), JsonObjectValue.JsonObject, Distribution, ParticleSystem, -1));
	}
}

void IParticleSystemImporter::EmptyParticleSystem() const {
	const auto ParticleSystem = GetTypedAsset<UParticleSystem>();
	
	if (ParticleSystem->Emitters.Num() > 0) {
		/* Destroy any existing emitters (optional, based on context) */
		for (UParticleEmitter* Emitter : ParticleSystem->Emitters) {
			if (Emitter) {
				Emitter->ConditionalBeginDestroy();
			}
		}

		/* Clear the emitters array */
		ParticleSystem->Emitters.Empty();
	}
}

void IParticleSystemImporter::CreateEmitters(TArray<TSharedPtr<FJsonValue>> Emitters) {
	const auto ParticleSystem = GetTypedAsset<UParticleSystem>();
	
	for (const TSharedPtr<FJsonValue>& EmitterReferenceValue : Emitters) {
		ParticleSystem->PreEditChange(nullptr);
		
		/* Properties & Information */
		auto EmitterReference = EmitterReferenceValue->AsObject();
		const auto Emitter_Object = AssetContainer.GetExportJsonObjectByObjectPath(EmitterReferenceValue->AsObject());

		auto Name = Emitter_Object->GetStringField(TEXT("Name"));
		auto EmitterProperties = Emitter_Object->GetObjectField(TEXT("Properties"));
		auto EmitterType = Emitter_Object->GetStringField(TEXT("Type"));

		const UClass* Class = FindObject<UClass>(ANY_PACKAGE, *EmitterType);

		CreateEmitter(Class, FName(*Name), EmitterProperties);
	}
}

UParticleEmitter* IParticleSystemImporter::CreateEmitter(const UClass* Class, const FName Name, const TSharedPtr<FJsonObject>& EmitterProperties) {
	const auto ParticleSystem = GetTypedAsset<UParticleSystem>();
	
	UParticleEmitter* Emitter = NewObject<UParticleEmitter>(ParticleSystem, Class, Name, RF_Transactional);

	/* Add to particle system */
	ParticleSystem->Emitters.Add(Emitter);

	/* Setup LODLevels ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	for (const TSharedPtr<FJsonValue>& EmitterLODReferenceLevel : EmitterProperties->GetArrayField(TEXT("LODLevels"))) {
		/* Properties & Information */
		auto EmitterLODReference = EmitterLODReferenceLevel->AsObject();
		const auto EmitterLODLevelObject = AssetContainer.GetExportJsonObjectByObjectPath(EmitterLODReference);

		/* Create and handle LODLevel */
		CreateLODLevel(EmitterLODLevelObject->GetObjectField(TEXT("Properties")), Emitter);
	}
	
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

	GetObjectSerializer()->DeserializeObjectProperties(RemovePropertiesShared(EmitterProperties,
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

UParticleLODLevel* IParticleSystemImporter::CreateLODLevel(const TSharedPtr<FJsonObject>& LevelProperties, UParticleEmitter* Emitter) {
	const auto ParticleSystem = GetTypedAsset<UParticleSystem>();
	
	/* LOD Level by default is zero */
	int Level = 0;

	if (LevelProperties->HasField(TEXT("Level"))) {
		Level = LevelProperties->GetIntegerField(TEXT("Level"));
	}

	/* Find the LOD Level if it already exists */
	UParticleLODLevel* LODLevel = Emitter->GetLODLevel(Level);

	/* Create a LOD Level if it doesn't exist */
	if (LODLevel == nullptr) {
		LODLevel = NewObject<UParticleLODLevel>(Emitter);

		check(LODLevel);

		LODLevel->Level = Level;
		if (LevelProperties->HasField(TEXT("bEnabled")))
		{
			LODLevel->bEnabled = LevelProperties->GetBoolField(TEXT("bEnabled"));
		}
		else
		{
			LODLevel->bEnabled = true;
		}
		
		LODLevel->ConvertedModules = true;
		LODLevel->PeakActiveParticles = 0;

		/* Create the RequiredModule */
		UParticleModuleRequired* RequiredModule = NewObject<UParticleModuleRequired>(ParticleSystem);
		check(RequiredModule);
		LODLevel->RequiredModule = RequiredModule;
		
		RequiredModule->ModuleEditorColor = FColor::MakeRandomColor();

		UParticleModuleSpawn* SpawnModule = NewObject<UParticleModuleSpawn>(ParticleSystem);
		check(SpawnModule);
		
		LODLevel->SpawnModule = SpawnModule;
		SpawnModule->BurstList.Empty();
		
		LODLevel->TypeDataModule = nullptr;
		Emitter->LODLevels.Add(LODLevel);
	}

	LODLevel->Modules.Empty();

	GetObjectSerializer()->DeserializeObjectProperties(RemovePropertiesShared(LevelProperties, {
		"RequiredModule",
		"Modules",
		"TypeDataModule",
		"SpawnModule"
	}), LODLevel);

	/* RequiredModule */
	if (LevelProperties->HasField(TEXT("RequiredModule"))) {
		auto& RequiredModule = LevelProperties->GetObjectField(TEXT("RequiredModule"));
		const auto RequiredModuleObject = AssetContainer.GetExportJsonObjectByObjectPath(RequiredModule);
		const auto LOD_RequiredModule = LODLevel->RequiredModule;

		SetModuleData(RequiredModuleObject->GetObjectField(TEXT("Properties")), LOD_RequiredModule);
	}

	/* SpawnModule */
	if (LevelProperties->HasField(TEXT("SpawnModule"))) {
		auto& SpawnModule = LevelProperties->GetObjectField(TEXT("SpawnModule"));
		const auto SpawnModuleObject = AssetContainer.GetExportJsonObjectByObjectPath(SpawnModule);

		SetModuleData(SpawnModuleObject->GetObjectField(TEXT("Properties")), LODLevel->SpawnModule);
	}

	/* TypeDataModule */
	if (LevelProperties->HasField(TEXT("TypeDataModule"))) {
		const TSharedPtr<FJsonObject> TypeDataModulePath = LevelProperties->GetObjectField(TEXT("TypeDataModule"));
		const TSharedPtr<FJsonObject> TypeDataModuleObject = AssetContainer.GetExportJsonObjectByObjectPath(TypeDataModulePath);

		const FString EmitterType = TypeDataModuleObject->GetStringField(TEXT("Type"));

		/* Find Emitter Class */
		const UClass* EmitterClass = FindObject<UClass>(ANY_PACKAGE, *EmitterType);

		UParticleModuleTypeDataBase* TypeDataModule = NewObject<UParticleModuleTypeDataBase>(ParticleSystem, EmitterClass);
		check(TypeDataModule);
		LODLevel->TypeDataModule = TypeDataModule;

		SetModuleData(TypeDataModuleObject->GetObjectField(TEXT("Properties")), TypeDataModule);
	}

	auto Modules = LevelProperties->GetArrayField(TEXT("Modules"));

	for (const TSharedPtr<FJsonValue>& ModulesReference : Modules) {
		/* Properties & Information */
		auto EmitterLODReference = ModulesReference->AsObject();
		const auto EmitterLODLevelObject = AssetContainer.GetExportJsonObjectByObjectPath(EmitterLODReference);

		auto EmitterType = EmitterLODLevelObject->GetStringField(TEXT("Type"));
		auto EmitterName = EmitterLODLevelObject->GetStringField(TEXT("Name"));

		/* Find Emitter Class */
		const UClass* EmitterClass = FindObject<UClass>(ANY_PACKAGE, *EmitterType);

		UParticleModule* Module = NewObject<UParticleModule>(ParticleSystem, EmitterClass, FName(*EmitterName));
		check(Module);

		SetModuleData(EmitterLODLevelObject->GetObjectField(TEXT("Properties")), Module);

		Module->ModuleEditorColor = FColor::MakeRandomColor();

		Module->SetTransactionFlag();
		
		LODLevel->Modules.Add(Module);

		ParticleSystem->PostEditChange();
		ParticleSystem->MarkPackageDirty();
	}

	Emitter->PostEditChange();
	Emitter->SetFlags(RF_Transactional);
	
	return nullptr;
}

void IParticleSystemImporter::SetModuleData(const TSharedPtr<FJsonObject>& ModuleProperties, UParticleModule* Module) const {
	GetObjectSerializer()->Parent = Module;

	GetObjectSerializer()->DeserializeObjectProperties(RemovePropertiesShared(ModuleProperties,
	{
		"RequiredModule",
		"Modules",
		"TypeDataModule",
		"SpawnModule",
		"Emitters"
	}), Module);

	Module->PostLoad();
}
