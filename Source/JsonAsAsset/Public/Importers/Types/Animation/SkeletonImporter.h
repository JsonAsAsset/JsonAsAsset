/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"

class ISkeletonImporter : public IImporter {
public:
	ISkeletonImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& JsonObjects):
		IImporter(JsonObject, Package, JsonObjects) {
	}

	virtual UObject* CreateAsset(UObject* CreatedAsset) override;
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