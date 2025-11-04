/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "JsonAsAssetSettings.generated.h"

/* Settings for materials */
USTRUCT()
struct FJMaterialImportSettings
{
	GENERATED_BODY()
public:
	/* Constructor to initialize default values */
	FJMaterialImportSettings()
		: bSkipResultNodeConnection(false)
	{}

	/**
	 * Prevents a known error during Material asset import/download:
	 * "Material expression called Compiler->TextureParameter() without implementing UMaterialExpression::GetReferencedTexture properly."
	 *
	 * To avoid this issue, this option skips connecting the inputs to the material's primary result node, potentially fixing the error.
	 *
	 * Usage:
	 *  - If enabled, import the material, save your project, restart the editor, and then re-import the material.
	 *  - Alternatively, manually connect the inputs to the main result node.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Material Import Settings")
	bool bSkipResultNodeConnection;
};

/* Settings for animation blueprints */
USTRUCT()
struct FJAnimationBlueprintImportSettings
{
	GENERATED_BODY()
public:
	/* Constructor to initialize default values */
	FJAnimationBlueprintImportSettings()
		: bShowAllNodeKeysAsComment(false)
	{}

	UPROPERTY(EditAnywhere, Config, AdvancedDisplay, Category = "Animation Blueprint Settings")
	bool bShowAllNodeKeysAsComment;
};

/* Settings for textures */
USTRUCT()
struct FJTextureImportSettings
{
	GENERATED_BODY()
public:
	/* Constructor to initialize default values */
	FJTextureImportSettings()
		: bForceRedownloadTextures(false)
	{}

	/**
	 * Enables re-downloading of textures even if they already exist.
	 */
	UPROPERTY(EditAnywhere, Config, AdvancedDisplay, DisplayName = "Force Redownload Textures", Category = "Texture Import Settings")
	bool bForceRedownloadTextures;
};

/* Settings for sounds */
USTRUCT()
struct FJSoundImportSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(Config)
	FString AudioFileExtension = "ogg";
};

/* Settings for pose assets */
USTRUCT()
struct FJPoseAssetImportSettings
{
	GENERATED_BODY()
};

USTRUCT()
struct FJPathRedirector
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Config, Category = "Path Redirector")
	FString Source;

	UPROPERTY(EditAnywhere, Config, Category = "Path Redirector")
	FString Target;
};

USTRUCT()
struct FJAssetSettings
{
	GENERATED_BODY()
public:
	/* Constructor to initialize default values */
	FJAssetSettings()
		: bUE5Target(false),
		  bSavePackagesOnImport(false)
	{
		MaterialImportSettings = FJMaterialImportSettings();
		SoundImportSettings = FJSoundImportSettings();
		TextureImportSettings = FJTextureImportSettings();
		AnimationBlueprintImportSettings = FJAnimationBlueprintImportSettings();
		PoseAssetImportSettings = FJPoseAssetImportSettings();
	}

	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	FJTextureImportSettings TextureImportSettings;
	
	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	FJMaterialImportSettings MaterialImportSettings;

	UPROPERTY(Config)
	FJSoundImportSettings SoundImportSettings;

	/* UPROPERTY(EditAnywhere, Config, Category = AssetSettings) */
	FJPoseAssetImportSettings PoseAssetImportSettings;

	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	FJAnimationBlueprintImportSettings AnimationBlueprintImportSettings;

	/* Game's Project Name (Set by Cloud) */
	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	FString GameName;

	/* If imported assets are from UE5. (Set by Cloud) */
	UPROPERTY(Config)
	bool bUE5Target;
	
	UPROPERTY(EditAnywhere, Config, Category = AssetSettings, meta = (DisplayName = "Save Assets On Import"))
	bool bSavePackagesOnImport;

	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	TArray<FJPathRedirector> PathRedirectors;
};

USTRUCT()
struct FJVersioningSettings
{
	GENERATED_BODY()
public:
	/* Disable checking for newer updates of JsonAsAsset. */
	UPROPERTY(EditAnywhere, Config, Category = VersioningSettings)
	bool bDisable = false;
	
	/* Enable update reminders for newer updates of JsonAsAsset. */
	UPROPERTY(EditAnywhere, Config, Category = VersioningSettings)
	bool bEnableReminders = true;
};

/* Powerful Unreal Engine Plugin that imports assets from FModel */
UCLASS(Config = EditorPerProjectUserSettings, DefaultConfig)
class JSONASASSET_API UJsonAsAssetSettings : public UDeveloperSettings {
	GENERATED_BODY()
public:
	UJsonAsAssetSettings();

#if WITH_EDITOR
	virtual FText GetSectionText() const override;
#endif

	/**
	 * Specifies the directory path for exported assets.
	 * (e.g. Output/Exports)
	 */
	UPROPERTY(Config)
	FDirectoryPath ExportDirectory;

	UPROPERTY(EditAnywhere, Config, Category = Configuration)
	FJVersioningSettings Versioning;
	
	UPROPERTY(EditAnywhere, Config, Category = Configuration)
	FJAssetSettings AssetSettings;

	/* Enables experimental/developing features of JsonAsAsset. Features may not work as intended. */
	UPROPERTY(EditAnywhere, Config, Category = Configuration, AdvancedDisplay)
	bool bEnableExperiments;

	/**
	 * Retrieves assets from an API and imports references directly into your project.
	 */
	UPROPERTY(EditAnywhere, Config, Category = Cloud, DisplayName = "Enable Cloud")
	bool bEnableCloudServer;

	static bool EnsureExportDirectoryIsValid(UJsonAsAssetSettings* Settings);

	static bool IsSetup(UJsonAsAssetSettings* Settings, TArray<FString>& Reasons) {
		const bool IsExportDirectoryValid = EnsureExportDirectoryIsValid(Settings);
		
		if (!IsExportDirectoryValid) {
			Reasons.Add("Export Directory is missing");
		}

		return IsExportDirectoryValid;
	}
	
	static bool IsSetup(UJsonAsAssetSettings* Settings) {
		if (Settings == nullptr) return false;
		
		TArray<FString> Params;
		return IsSetup(Settings, Params);
	}
};