/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MaterialSettings.generated.h"

/* Settings for materials */
USTRUCT()
struct FJMaterialSettings
{
	GENERATED_BODY()
public:
	/**
	 * Prevents a known error:
	 * "Material expression called Compiler->TextureParameter() without implementing UMaterialExpression::GetReferencedTexture properly."
	 *
	 * To avoid this issue, this option skips connecting the inputs to the material's primary result node, potentially fixing the error.
	 *
	 * Usage:
	 *  - If enabled, import the material, save your project, restart the editor, and then re-import the material.
	 *  - Alternatively, manually connect the inputs to the main result node.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Material Import Settings")
	bool bSkipResultNodeConnection = false;
};