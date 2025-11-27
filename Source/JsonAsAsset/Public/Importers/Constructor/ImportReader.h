/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once
#include "Dom/JsonValue.h"

class JSONASASSET_API IImportReader {
public:
	static bool ReadExportsAndImport(TArray<TSharedPtr<FJsonValue>> Exports, const FString& File, bool bHideNotifications = false);
	static void ReadExportAndImport(const TArray<TSharedPtr<FJsonValue>>& Exports, const TSharedPtr<FJsonObject>& Export, FString File, bool bHideNotifications = false);
	static void ImportReference(const FString& File);
};
