﻿/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"

class IPoseAssetImporter : public IImporter {
public:
	IPoseAssetImporter(const FString& FileName, const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, UPackage* OutermostPkg, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects, UClass* AssetClass):
		IImporter(FileName, FilePath, JsonObject, Package, OutermostPkg, AllJsonObjects, AssetClass) {
	}

	virtual bool Import() override;
	void ReverseCookLocalSpacePose(USkeleton* Skeleton) const;
};

REGISTER_IMPORTER(IPoseAssetImporter, TArray<FString>{ 
	TEXT("PoseAsset")
}, TEXT("Animation Assets"));