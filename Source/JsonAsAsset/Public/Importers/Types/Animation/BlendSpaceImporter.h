/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"

class IBlendSpaceImporter : public IImporter {
public:
	IBlendSpaceImporter(const FString& AssetName, const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, UPackage* OutermostPkg, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects, UClass* AssetClass):
		IImporter(AssetName, FilePath, JsonObject, Package, OutermostPkg, AllJsonObjects, AssetClass) {
	}

	virtual bool Import() override;
};

REGISTER_IMPORTER(IBlendSpaceImporter, (TArray<FString>{
	"BlendSpace",
	"BlendSpace1D",

	"AimOffsetBlendSpace",
	"AimOffsetBlendSpace1D"
}), TEXT("Animation Assets"));