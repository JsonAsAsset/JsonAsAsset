/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

/* Style of JsonAsAsset */
class FJsonAsAssetStyle {
public:
	static const ISlateStyle& Get();
	static FName GetStyleSetName();
	
public:
	static void Initialize();
	static void ReloadTextures();

private:
	static TSharedRef<FSlateStyleSet> Create();
	static TSharedPtr<FSlateStyleSet> StyleInstance;

public:
	static void Shutdown();
};