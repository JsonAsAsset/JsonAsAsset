/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Graph/MaterialGraph.h"

class IMaterialFunctionImporter : public IMaterialGraph {
public:
	IMaterialFunctionImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& JsonObjects):
		IMaterialGraph(JsonObject, Package, JsonObjects) {
	}

	virtual UObject* CreateAsset(UObject* CreatedAsset) override;
	virtual bool Import() override;
};

REGISTER_IMPORTER(IMaterialFunctionImporter, {
	"MaterialFunction"
}, "Material Assets");