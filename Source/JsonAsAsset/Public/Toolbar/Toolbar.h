/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Utilities/Compatibility.h"

struct FJsonAsAssetToolbar {
	/*
	 * Registers the JsonAsAsset button onto the toolbar
	 *
	 * NOTE: Called on FJsonAsAssetModule::StartupModule
	 */
	void Register(FName MenuToExtend = "LevelEditor.LevelEditorToolBar.PlayToolBar");

#if ENGINE_UE4
	void UE4Register(FToolBarBuilder& Builder);
#endif
	
	bool IsActionEnabled() const;
	FText GetTooltipText() const;
	
	/* Checks if JsonAsAsset is fit to function, and opens a JSON file dialog */
	void ImportAction();
	
	TSharedRef<SWidget> CreateMenuDropdown();
};
