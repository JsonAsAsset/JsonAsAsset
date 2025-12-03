/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

/* Settings Substructures */
#include "Types/AnimationBlueprintSettings.h"
#include "Types/MaterialSettings.h"
#include "Types/TextureSettings.h"
#include "Runtime.h"
#include "Redirector.h"

#include "JsonAsAssetSettings.generated.h"

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

	/* Game's Project Name */
	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	FString GameName;
	
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
class JSONASASSET_API UJsonAsAssetSettings : public UDeveloperSettings {
	GENERATED_BODY()
public:
	UJsonAsAssetSettings();

protected:
#if WITH_EDITOR
	/** Gets the section text. */
	virtual FText GetSectionText() const override;
#endif

public:
	static bool EnsureExportDirectoryIsValid(UJsonAsAssetSettings* Settings);

	static bool IsSetup(UJsonAsAssetSettings* Settings, TArray<FString>& Reasons);
	static bool IsSetup(UJsonAsAssetSettings* Settings);

	static void ReadAppData();
public:
	UPROPERTY(Config)
	FJRuntime Runtime;
	
	UPROPERTY(EditAnywhere, Config, Category = Configuration)
	FJVersioningSettings Versioning;
	
	UPROPERTY(EditAnywhere, Config, Category = Configuration)
	FJSettings AssetSettings;

	UPROPERTY(EditAnywhere, Config, Category = Redirectors)
	TArray<FJRedirector> Redirectors;

	/* Retrieves assets from an API and imports references directly into your project. */
	UPROPERTY(EditAnywhere, Config, Category = Cloud, DisplayName = "Enable Cloud")
	bool bEnableCloudServer = true;

	/* Enables experimental/developing features of JsonAsAsset. Features may not work as intended. */
	UPROPERTY(EditAnywhere, Config, Category = Configuration, AdvancedDisplay)
	bool bEnableExperiments = false;
};