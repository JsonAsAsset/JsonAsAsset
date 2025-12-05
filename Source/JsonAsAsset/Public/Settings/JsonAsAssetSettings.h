/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

/* Settings Substructures */
#include "Types/AnimationBlueprintSettings.h"
#include "Types/MaterialSettings.h"
#include "Types/TextureSettings.h"
#include "Redirector.h"
#include "Interfaces/Validation.h"

#include "JsonAsAssetSettings.generated.h"

extern FName GJsonAsAssetName;
extern FName GJsonAsAssetSettingsCategoryName;

USTRUCT()
struct FJSettings
{
	GENERATED_BODY()
public:
	/* Constructor to initialize default values */
	FJSettings()
	{
		Material = FJMaterialSettings();
		Texture = FJTextureSettings();
		AnimationBlueprint = FJAnimationBlueprintSettings();
	}

	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	FJAnimationBlueprintSettings AnimationBlueprint;

	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	FJTextureSettings Texture;
	
	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	FJMaterialSettings Material;

	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	FString ProjectName;
	
	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	bool bSaveAssets = false;
};

USTRUCT()
struct FJVersioningSettings
{
	GENERATED_BODY()
public:
	/* Disable checking for newer updates of JsonAsAsset. */
	UPROPERTY(EditAnywhere, Config, Category = VersioningSettings)
	bool bDisable = false;
};

/* Powerful Unreal Engine Plugin that imports assets from FModel */
UCLASS(Config = EditorPerProjectUserSettings, DefaultConfig)
class JSONASASSET_API UJsonAsAssetSettings : public UDeveloperSettings, public IJsonAsAssetValidationInterface {
	GENERATED_BODY()
public:
	UJsonAsAssetSettings();

	/* Overriden to stop the Editor spacing the words between JsonAsAsset */
	virtual FText GetSectionText() const override;
	
public:
	UPROPERTY(EditAnywhere, Config, Category = Settings)
	FJVersioningSettings Versioning;
	
	UPROPERTY(EditAnywhere, Config, Category = Settings)
	FJSettings AssetSettings;

	UPROPERTY(EditAnywhere, Config, Category = Redirectors, meta = (TitleProperty = "Name"))
	TArray<FJRedirector> Redirectors;

	/* Retrieves assets from an API and imports references directly into your project. */
	UPROPERTY(EditAnywhere, Config, Category = Cloud, DisplayName = "Enable Cloud")
	bool bEnableCloudServer = true;

	/* Enables experimental/developing features of JsonAsAsset. Features may not work as intended. */
	UPROPERTY(EditAnywhere, Config, Category = Settings, AdvancedDisplay)
	bool bEnableExperiments = false;
};