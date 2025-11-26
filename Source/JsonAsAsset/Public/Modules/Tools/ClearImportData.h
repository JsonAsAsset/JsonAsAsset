/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "ToolBase.h"
#include "ClearImportData.generated.h"

USTRUCT()
struct FToolClearImportData : public FToolBase {
	GENERATED_BODY()
public:
	static void Execute();
};