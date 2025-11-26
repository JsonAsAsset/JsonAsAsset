/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/Tools/ToolBase.h"
#include "AnimationData.generated.h"

USTRUCT()
struct FToolAnimationData : public FToolBase {
	GENERATED_BODY()
public:
	static void Execute();
};