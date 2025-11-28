/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"

class IBlendSpaceImporter : public IImporter {
public:
	IBlendSpaceImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& JsonObjects):
		IImporter(JsonObject, Package, JsonObjects) {
	}

	virtual UObject* CreateAsset(UObject* CreatedAsset) override;
	virtual bool Import() override;
};

REGISTER_IMPORTER(IBlendSpaceImporter, (TArray<FString>{
	"BlendSpace",
	"BlendSpace1D",

	"AimOffsetBlendSpace",
	"AimOffsetBlendSpace1D"
}), TEXT("Animation Assets"));