/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"

class ISkeletonImporter : public IImporter {
public:
	ISkeletonImporter(const FString& AssetName, const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, UPackage* OutermostPackage, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects, UClass* AssetClass):
		IImporter(AssetName, FilePath, JsonObject, Package, OutermostPackage, AllJsonObjects, AssetClass) {
	}

	virtual bool Import() override;

protected:
	virtual void ApplyModifications() override;
	void ApplySkeletalChanges(USkeleton* Skeleton) const;
	void ApplySkeletalAssetData(USkeleton* Skeleton) const;

	static void RebuildSkeleton(const USkeleton* Skeleton);
private:
	void DeserializeCurveMetaData(FCurveMetaData* OutMeta, const TSharedPtr<FJsonObject>& Json) const;
};

REGISTER_IMPORTER(ISkeletonImporter, {
	"Skeleton"
}, "Skeletal Assets");