/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Dom/JsonValue.h"
#include "Importers/Constructor/Types.h"
#include "Settings/JsonAsAssetSettings.h"

class IImporter;

/* Easy way to find importers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
using FImporterFactoryDelegate = TFunction<IImporter*(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& Exports)>;

/* Registration info for an importer */
struct FImporterRegistrationInfo {
	FString Category;
	FImporterFactoryDelegate Factory;

	FImporterRegistrationInfo(const FString& InCategory, const FImporterFactoryDelegate& InFactory)
		: Category(InCategory)
		, Factory(InFactory)
	{
	}

	FImporterRegistrationInfo() = default;
};

inline TMap<TArray<FString>, FImporterRegistrationInfo>& GetFactoryRegistry() {
	static TMap<TArray<FString>, FImporterRegistrationInfo> Registry;
        
	return Registry;
}

inline FImporterFactoryDelegate* FindFactoryForAssetType(const FString& AssetType) {
	const UJsonAsAssetSettings* Settings = GetSettings();

	for (auto& Pair : GetFactoryRegistry()) {
		if (!Settings->bEnableExperiments) {
			if (ImportTypes::Experimental.Contains(AssetType)) return nullptr;
		}
            
		if (Pair.Key.Contains(AssetType)) {
			return &Pair.Value.Factory;
		}
	}
        
	return nullptr;
}

template <typename T>
IImporter* CreateImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& Exports) {
	return new T(JsonObject, Package, Exports);
}