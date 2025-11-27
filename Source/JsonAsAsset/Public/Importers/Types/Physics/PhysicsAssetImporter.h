/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"

#include "PhysicsEngine/PhysicsAsset.h"

/* SkeletalBodySetup is a separate file on UE5.5+ */
#if ENGINE_UE5 && ENGINE_MINOR_VERSION > 4
#include "PhysicsEngine/SkeletalBodySetup.h"
#endif

class IPhysicsAssetImporter : public IImporter {
public:
	IPhysicsAssetImporter(const FString& AssetName, const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, UPackage* OutermostPackage, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects, UClass* AssetClass):
		IImporter(AssetName, FilePath, JsonObject, Package, OutermostPackage, AllJsonObjects, AssetClass) {
	}

	virtual UObject* CreateAsset(UObject* CreatedAsset = nullptr) override;
	virtual bool Import() override;

	static USkeletalBodySetup* CreateNewBody(UPhysicsAsset* PhysAsset, FName ExportName, FName BoneName);
	static UPhysicsConstraintTemplate* CreateNewConstraint(UPhysicsAsset* PhysAsset, FName ExportName);
};

REGISTER_IMPORTER(IPhysicsAssetImporter, {
	"PhysicsAsset"
}, "Physics Assets");