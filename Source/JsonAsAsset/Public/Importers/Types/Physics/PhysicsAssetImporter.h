// Copyright JAA Contributors 2024-2025

#pragma once

#include "Importers/Constructor/Importer.h"
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 4
#include "PhysicsEngine/PhysicsConstraintTemplate.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#endif

class IPhysicsAssetImporter : public IImporter {
public:
	IPhysicsAssetImporter(const FString& FileName, const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, UPackage* OutermostPkg, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects):
		IImporter(FileName, FilePath, JsonObject, Package, OutermostPkg, AllJsonObjects) {
	}
	
	virtual bool Import() override;

	static USkeletalBodySetup* CreateNewBody(UPhysicsAsset* PhysAsset, FName ExportName, FName BoneName);
	static UPhysicsConstraintTemplate* CreateNewConstraint(UPhysicsAsset* PhysAsset, FName ExportName);
};
