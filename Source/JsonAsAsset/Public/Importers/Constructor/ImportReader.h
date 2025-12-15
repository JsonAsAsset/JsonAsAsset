/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Dom/JsonValue.h"
#include "Utilities/Serializers/Containers/ObjectExport.h"

class JSONASASSET_API IImportReader {
public:
	static bool ReadExportsAndImport(const TArray<TSharedPtr<FJsonValue>>& Exports, const FString& File, bool bHideNotifications = false);
	static void ReadExportAndImport(FUObjectExportContainer& Container, FUObjectExport& Export, FString File, bool bHideNotifications = false);
	static void ImportReference(const FString& File);
};
