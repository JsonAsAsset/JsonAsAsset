/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/Tools/ToolBase.h"
#include "ConvexCollision.generated.h"

USTRUCT()
struct FToolConvexCollision : public FToolBase {
	GENERATED_BODY()
public:
	static void Execute();
};