/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"

class IMaterialInstanceConstantImporter : public IImporter {
public:
	IMaterialInstanceConstantImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& JsonObjects):
		IImporter(JsonObject, Package, JsonObjects) {
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