/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "./Importer.h"

/* Basic template importer using Asset Class. */
template <typename AssetType>
class ITemplatedImporter : public IImporter {
public:
	ITemplatedImporter(const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, UPackage* OutermostPackage, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects):
		IImporter(FilePath, JsonObject, Package, OutermostPackage, AllJsonObjects) {
	}

	virtual UObject* CreateAsset(UObject* CreatedAsset) override;
	virtual bool Import() override;
};