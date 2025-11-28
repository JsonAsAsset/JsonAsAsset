/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"

class IAnimationBaseImporter : public IImporter {
public:
	IAnimationBaseImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& JsonObjects):
		IImporter(JsonObject, Package, JsonObjects) {
	}

	virtual bool Import() override;
};

REGISTER_IMPORTER(IAnimationBaseImporter, (TArray<FString>{ 
	TEXT("AnimSequence"),
	TEXT("AnimMontage")
}), TEXT("Animation Assets"));