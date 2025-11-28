/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Types.h"
#include "Registry/RegistrationInfo.h"
#include "Utilities/Compatibility.h"

inline bool CanImport(const FString& Type, const bool IsCloud = false, const UClass* Class = nullptr) {
	if (IsCloud) {
		if (!ImportTypes::Cloud::Allowed(Type)) {
			return false;
		}
	}
    
	if (FindFactoryForAssetType(Type)) {
		return true;
	}
    
	for (const TPair<FString, TArray<FString>>& Pair : ImportTypes::Templated) {
		if (Pair.Value.Contains(Type)) {
			return true;
		}
	}

	if (!Class) {
		Class = FindClassByType(Type);
	}

	if (Class == nullptr) return false;

	/* ?? */
	if (Type == "MaterialInterface") return true;

	if (ImportTypes::Cloud::Extra.Contains(Type)) {
		return true;
	}

	if (!ImportTypes::Allowed(Type)) return false;

	return Class->IsChildOf(UDataAsset::StaticClass());
}