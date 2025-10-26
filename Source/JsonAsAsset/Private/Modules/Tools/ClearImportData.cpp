/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Modules/Tools/ClearImportData.h"
#include "EditorFramework/AssetImportData.h"
#include "Utilities/EngineUtilities.h"

void FToolClearImportData::Execute() {
	TArray<FAssetData> AssetDataList = GetAssetsInSelectedFolder();

	if (AssetDataList.Num() == 0) {
		return;
	}

	static const TArray<FName> SupportedClasses = {
		"AnimSequence",
		"SkeletalMesh",
		"StaticMesh",
		"Texture"
	};

	for (const FAssetData& AssetData : AssetDataList) {
		if (!AssetData.IsValid() || !SupportedClasses.Contains(AssetData.AssetClass)) {
			continue;
		}
		
		UObject* Asset = AssetData.GetAsset();
		if (Asset == nullptr) continue;

		if (UAnimSequence* AnimSequence = Cast<UAnimSequence>(Asset)) {
			AnimSequence->AssetImportData->SourceData.SourceFiles.Empty();
			AnimSequence->Modify();
		}

		if (const UStaticMesh* StaticMesh = Cast<UStaticMesh>(Asset)) {
			StaticMesh->AssetImportData->SourceData.SourceFiles.Empty();
		}

		if (const UTexture* Texture = Cast<UTexture>(Asset)) {
			Texture->AssetImportData->SourceData.SourceFiles.Empty();
		}

		if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(Asset)) {
#if UE4_26
			SkeletalMesh->AssetImportData = nullptr;
#else
			SkeletalMesh->SetAssetImportData(nullptr);
#endif
			SkeletalMesh->Modify();
		}

		Asset->Modify();
	}
}
