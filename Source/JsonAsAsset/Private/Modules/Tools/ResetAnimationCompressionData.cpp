/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Modules/Tools/ResetAnimationCompressionData.h"
#include "Utilities/EngineUtilities.h"

void TToolResetAnimationCompressionData::Execute() {
	TArray<FAssetData> AssetDataList = GetAssetsInSelectedFolder();

	if (AssetDataList.Num() == 0) {
		return;
	}

	static const TArray<FName> SupportedClasses = {
		"AnimSequence"
	};

	for (const FAssetData& AssetData : AssetDataList) {
		if (!AssetData.IsValid() || !SupportedClasses.Contains(GetAssetDataClass(AssetData))) {
			continue;
		}
		
		UObject* Asset = AssetData.GetAsset();
		if (Asset == nullptr) continue;

		if (UAnimSequence* AnimSequence = Cast<UAnimSequence>(Asset)) {
			AnimSequence->BoneCompressionSettings = nullptr;
			AnimSequence->CurveCompressionSettings = nullptr;

#if ENGINE_UE5
			AnimSequence->BeginCacheDerivedDataForCurrentPlatform();
#endif
			AnimSequence->Modify();
		}

		Asset->Modify();
	}
}
