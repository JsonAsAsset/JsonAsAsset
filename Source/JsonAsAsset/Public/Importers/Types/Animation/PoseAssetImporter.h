/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"

class IPoseAssetImporter final : public IImporter {
public:
	IPoseAssetImporter(const FString& AssetName, const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, UPackage* OutermostPkg, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects, UClass* AssetClass):
		IImporter(AssetName, FilePath, JsonObject, Package, OutermostPkg, AllJsonObjects, AssetClass), PoseAsset(nullptr) {
	}

	UPoseAsset* PoseAsset;

	virtual bool Import() override;

protected:
	void ReverseCookLocalSpacePose(USkeleton* Skeleton) const;
	static UAnimSequence* CreateAnimSequenceFromPose(USkeleton* Skeleton, const FString& SequenceName, const TSharedPtr<FJsonObject>& PoseContainer, UPackage* Outer);
};

REGISTER_IMPORTER(IPoseAssetImporter, TArray<FString>{ 
	TEXT("PoseAsset")
}, TEXT("Animation Assets"));