/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

/* AssetType/Category ~ Defined in CPP */
extern TMap<FString, TArray<FString>> ImporterTemplatedTypes;

inline TArray<FString> BlacklistedCloudTypes = {
	"AnimSequence",
	"AnimMontage",
	"AnimBlueprintGeneratedClass"
};

inline TArray<FString> ExtraCloudTypes = {
	"TextureLightProfile"
};

inline const TArray<FString> ExperimentalAssetTypes = {
	"AnimBlueprintGeneratedClass"
};