/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

struct ImportTypes {
	/* AssetType/Category ~ Defined in CPP */
	static TMap<FString, TArray<FString>> Templated;
	
	struct Cloud {
		static inline TArray<FString> Blacklisted = {
			"AnimSequence",
			"AnimMontage",
			"AnimBlueprintGeneratedClass"
		};

		static inline TArray<FString> Extra = {
			"TextureLightProfile"
		};

		static bool Allowed(const FString& Type);
	};

	static inline TArray<FString> Experimental = {
		"AnimBlueprintGeneratedClass"
	};

	static bool Allowed(const FString& ImporterType);
};