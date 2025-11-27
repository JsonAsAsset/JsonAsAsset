/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Graph/MaterialGraph.h"

class IMaterialImporter final : public IMaterialGraph {
public:
	IMaterialImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects):
		IMaterialGraph(JsonObject, Package, AllJsonObjects) {
	}

	virtual UObject* CreateAsset(UObject* CreatedAsset) override;
	virtual bool Import() override;
};

REGISTER_IMPORTER(IMaterialImporter, {
	"Material"
}, "Material Assets");