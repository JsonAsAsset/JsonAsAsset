/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/Animation/BlendSpaceImporter.h"
#include "Animation/BlendSpace.h"

UObject* IBlendSpaceImporter::CreateAsset(UObject* CreatedAsset) {
#if ENGINE_UE5
	auto BlendSpace = NewObject<UBlendSpace>(Package, GetAssetClass(), *AssetName, RF_Public | RF_Standalone);
#else
	UBlendSpaceBase* BlendSpace = NewObject<UBlendSpaceBase>(Package, GetAssetClass(), *GetAssetName(), RF_Public | RF_Standalone);
#endif
	
	return IImporter::CreateAsset(BlendSpace);
}

bool IBlendSpaceImporter::Import() {
	UBlendSpaceBase* BlendSpace = Create<UBlendSpaceBase>();
	
	BlendSpace->Modify();
	
	GetObjectSerializer()->DeserializeObjectProperties(GetAssetData(), BlendSpace);

	/* Ensure internal state is refreshed after adding all samples */
	BlendSpace->ValidateSampleData();
	BlendSpace->MarkPackageDirty();
	BlendSpace->PostEditChange();
	BlendSpace->PostLoad();

	return OnAssetCreation(BlendSpace);
}