/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Redirector.generated.h"

USTRUCT()
struct FJRedirector
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Config, Category = PathRedirector)
	FString Source;

	UPROPERTY(EditAnywhere, Config, Category = PathRedirector)
	FString Target;
};