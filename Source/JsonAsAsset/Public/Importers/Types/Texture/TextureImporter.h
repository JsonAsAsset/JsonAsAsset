/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"

/* Basic template importer using Asset Class. */
template <typename AssetType>
class ITextureImporter : public IImporter {
public:
	ITextureImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects):
		IImporter(JsonObject, Package, AllJsonObjects) {
	}

	virtual bool Import() override;
};