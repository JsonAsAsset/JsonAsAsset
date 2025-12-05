/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AnimationBlueprintSettings.generated.h"

/* Settings for animation blueprints */
USTRUCT()
struct FJAnimationBlueprintSettings
{
	GENERATED_BODY()
public:
	/* Saves Keys/IDs in Node's comment. */
	UPROPERTY(EditAnywhere, Config, AdvancedDisplay, Category = AnimationBlueprint)
	bool bSaveNodeKeysInComments = false;
};