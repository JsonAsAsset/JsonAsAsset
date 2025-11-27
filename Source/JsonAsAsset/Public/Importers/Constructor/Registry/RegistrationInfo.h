/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Dom/JsonValue.h"
#include "Importers/Constructor/Types.h"
#include "Settings/JsonAsAssetSettings.h"

class IImporter;

/* Easy way to find importers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
using FImporterFactoryDelegate = TFunction<IImporter*(const FString& AssetName, const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, UPackage* OutermostPackage, const TArray<TSharedPtr<FJsonValue>>& Exports, UClass* AssetClass)>;

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

static TMap<TArray<FString>, FImporterRegistrationInfo>& GetFactoryRegistry() {
	static TMap<TArray<FString>, FImporterRegistrationInfo> Registry;
        
	return Registry;
}

static FImporterFactoryDelegate* FindFactoryForAssetType(const FString& AssetType) {
	const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();

	for (auto& Pair : GetFactoryRegistry()) {
		if (!Settings->bEnableExperiments) {
			if (ExperimentalAssetTypes.Contains(AssetType)) return nullptr;
		}
            
		if (Pair.Key.Contains(AssetType)) {
			return &Pair.Value.Factory;
		}
	}
        
	return nullptr;
}

template <typename T>
static IImporter* CreateImporter(const FString& AssetName, const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, UPackage* OutermostPackage, const TArray<TSharedPtr<FJsonValue>>& Exports, UClass* AssetClass) {
	return new T(AssetName, FilePath, JsonObject, Package, OutermostPackage, Exports, AssetClass);
}