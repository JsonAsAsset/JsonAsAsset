/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importer.h"
#include "Dom/JsonValue.h"
#include "Utilities/Serializers/Containers/ObjectExport.h"

class JSONASASSET_API IImportReader {
public:
	static bool ReadExportsAndImport(const TArray<TSharedPtr<FJsonValue>>& Exports, const FString& File, IImporter*& OutImporter, bool bHideNotifications = false);
	static IImporter* ReadExportAndImport(FUObjectExportContainer& Container, FUObjectExport& Export, FString File, bool bHideNotifications = false);
	static IImporter* ImportReference(const FString& File);
};
