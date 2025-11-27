/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"
#include "Engine/CurveTable.h"

class ICurveTableImporter : public IImporter {
public:
	ICurveTableImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects):
		IImporter(JsonObject, Package, AllJsonObjects) {
	}

	virtual UObject* CreateAsset(UObject* CreatedAsset) override;
	virtual bool Import() override;
};

REGISTER_IMPORTER(ICurveTableImporter, {
	"CurveTable"
}, "Table Assets");