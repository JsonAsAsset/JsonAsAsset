/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "./Importer.h"

/* Basic template importer using Asset Class. */
template <typename AssetType>
class ITemplatedImporter : public IImporter {
public:
	ITemplatedImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& JsonObjects):
		IImporter(JsonObject, Package, JsonObjects) {
	}

	virtual UObject* CreateAsset(UObject* CreatedAsset) override;
	virtual bool Import() override;
};