/* Copyright JsonAsAsset Contributors 2024-2026 */

#pragma once

#include "CoreMinimal.h"
#include "Utilities/Compatibility.h"

struct JSONASASSET_API FJsonAsAssetToolbar {
	void Register();
	void AddCloudButtons(FToolMenuSection& Section);
	
#if ENGINE_UE4
	static void UE4Register(FToolBarBuilder& Builder);
#endif

	/* Checks if JsonAsAsset is fit to function */
	static bool IsFitToFunction();
	
	/* Checks if JsonAsAsset is fit to function, and opens a JSON file dialog */
	static void ImportAction();

	/* UI Display ~~~~~~~~~~~~~~ */
	static TSharedRef<SWidget> CreateMenuDropdown();
	static TSharedRef<SWidget> CreateCloudMenuDropdown();
	
	static bool IsToolBarVisible();
};
