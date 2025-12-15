/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"
#include "UObject/StructOnScope.h"

class IDataTableImporter : public IImporter {
public:
	using FTableRowMap = TMap<FName, TSharedPtr<FStructOnScope>>;

	virtual UObject* CreateAsset(UObject* CreatedAsset) override;
	virtual bool Import() override;
};

REGISTER_IMPORTER(IDataTableImporter, {
	"DataTable"
}, "Table Assets");