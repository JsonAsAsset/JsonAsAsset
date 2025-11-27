/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"

class IMaterialInstanceConstantImporter : public IImporter {
public:
	IMaterialInstanceConstantImporter(const FString& AssetName, const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, UPackage* OutermostPackage, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects, UClass* AssetClass):
		IImporter(AssetName, FilePath, JsonObject, Package, OutermostPackage, AllJsonObjects, AssetClass) {
	}

	virtual UObject* CreateAsset(UObject* CreatedAsset) override;
	virtual bool Import() override;
protected:
	static void ReadStaticParameters(const TSharedPtr<FJsonObject>& StaticParameters, TArray<TSharedPtr<FJsonValue>>& StaticSwitchParameters, TArray<TSharedPtr<FJsonValue>>& StaticComponentMaskParameters);
	static void ConvertParameterNamesToInfos(TArray<TSharedPtr<FJsonValue>>& Input);
};

REGISTER_IMPORTER(IMaterialInstanceConstantImporter, {
	"MaterialInstanceConstant"
}, "Material Assets");