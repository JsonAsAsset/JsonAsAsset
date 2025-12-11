/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"

extern FName GJsonAsAssetName;

/* Caches basic metadata about JsonAsAsset */
struct FJMetadata {
	static TSharedPtr<IPlugin> Plugin;
	static FString Version;

	static void Initialize();
};