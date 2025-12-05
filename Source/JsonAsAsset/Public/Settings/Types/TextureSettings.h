/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TextureSettings.generated.h"

/* Settings for textures */
USTRUCT()
struct FJTextureSettings
{
	GENERATED_BODY()
public:
	/* Re-download textures that already exist. */
	UPROPERTY(EditAnywhere, Config, AdvancedDisplay, DisplayName = "Redownload Textures", Category = Texture)
	bool bReDownloadTextures = false;
};