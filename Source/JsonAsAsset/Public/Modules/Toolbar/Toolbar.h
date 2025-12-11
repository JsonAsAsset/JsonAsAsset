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
	void ImportAction();

	/* UI Display ~~~~~~~~~~~~~~ */
	TSharedRef<SWidget> CreateMenuDropdown();
	
	static bool IsToolBarVisible();
	bool IsActionEnabled() const;
	FText GetTooltipText() const;
};
