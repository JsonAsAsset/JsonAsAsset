/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once
#include "Types.h"
#include "Registry/RegistrationInfo.h"
#include "Utilities/Compatibility.h"

inline bool CanImportWithCloud(const FString& ImporterType) {
	if (BlacklistedCloudTypes.Contains(ImporterType)) {
		return false;
	}

	if (ExtraCloudTypes.Contains(ImporterType)) {
		return true;
	}

	return true;
}

inline bool IsAssetTypeImportableUsingCloud(const FString& ImporterType) {
	if (ExtraCloudTypes.Contains(ImporterType)) {
		return true;
	}

	return false;
}

inline bool CanImport(const FString& ImporterType, const bool IsCloud = false, const UClass* Class = nullptr) {
	/* Blacklists for Cloud importing */
	if (IsCloud) {
		if (!CanImportWithCloud(ImporterType)) {
			return false;
		}
	}
    
	if (FindFactoryForAssetType(ImporterType)) {
		return true;
	}
    
	for (const TPair<FString, TArray<FString>>& Pair : ImporterTemplatedTypes) {
		if (Pair.Value.Contains(ImporterType)) {
			return true;
		}
	}

	if (!Class) {
		Class = FindClassByType(ImporterType);
	}

	if (Class == nullptr) return false;

	if (ImporterType == "MaterialInterface") return true;

	if (IsAssetTypeImportableUsingCloud(ImporterType)) {
		return true;
	}
    
	return Class->IsChildOf(UDataAsset::StaticClass());
}

inline bool IsAssetTypeExperimental(const FString& ImporterType) {
	if (ExperimentalAssetTypes.Contains(ImporterType)) {
		return false;
	}

	return true;
}