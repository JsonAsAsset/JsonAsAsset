/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"

class ICurveVectorImporter : public IImporter {
public:
	ICurveVectorImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects):
		IImporter(JsonObject, Package, AllJsonObjects) {
	}

	virtual UObject* CreateAsset(UObject* CreatedAsset) override;
	virtual bool Import() override;
};

REGISTER_IMPORTER(ICurveVectorImporter, {
	"CurveVector"
}, "Curve Assets");