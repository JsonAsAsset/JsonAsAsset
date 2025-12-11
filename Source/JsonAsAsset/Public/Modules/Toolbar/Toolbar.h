/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Utilities/Compatibility.h"

struct JSONASASSET_API FJsonAsAssetToolbar {
	void Register();
	
#if ENGINE_UE4
	void UE4Register(FToolBarBuilder& Builder);
#endif
	
	/* Checks if JsonAsAsset is fit to function, and opens a JSON file dialog */
	static void ImportAction();

	/* UI Display ~~~~~~~~~~~~~~ */
	static TSharedRef<SWidget> CreateMenuDropdown();
	
	static bool IsToolBarVisible();
	static bool IsActionEnabled();
	FText GetTooltipText();
};
