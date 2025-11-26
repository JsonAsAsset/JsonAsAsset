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
	UPROPERTY(EditAnywhere, Config, AdvancedDisplay, Category = AnimationBlueprint)
	bool bDisplayNodeKeys = false;
};