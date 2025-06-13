/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Engine/DeveloperSettings.h"
#include "Modules/LocalFetchModule.h"
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
		: bDownloadExistingTextures(false)
	{}

	/**
	 * Enables re-downloading of textures even if they already exist in the Unreal Engine project.
	 *
	 * Use Case:
	 * This option is useful when you need to re-import textures that have been updated in a newer version of your Local Fetch build.
	 */
	UPROPERTY(EditAnywhere, Config, AdvancedDisplay, Category = "Texture Import Settings")
	bool bDownloadExistingTextures;
};

/* Settings for sounds */
USTRUCT()
struct FJSoundImportSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Config, Category = "Sound Import Settings", meta = (DisplayName = "Audio File Name Extension"))
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
struct FAssetSettings
{
	GENERATED_BODY()
public:
	/* Constructor to initialize default values */
	FAssetSettings()
		: bSavePackagesOnImport(false)
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

	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	FJSoundImportSettings SoundImportSettings;

	/* UPROPERTY(EditAnywhere, Config, Category = AssetSettings) */
	FJPoseAssetImportSettings PoseAssetImportSettings;

	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	FJAnimationBlueprintImportSettings AnimationBlueprintImportSettings;

	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	TArray<FJPathRedirector> PathRedirectors;

	UPROPERTY(EditAnywhere, Config, Category = AssetSettings, meta = (DisplayName = "Save Assets On Import"))
	bool bSavePackagesOnImport;

	/* Unreal Engine Game's Project Name (Set by Local Fetch) */
	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	FString GameName;
};

/* A user-friendly Unreal Engine plugin designed to import assets from packaged games through JSON files */
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
	 * (e.g., Output/Exports)
	 */
	UPROPERTY(EditAnywhere, Config, Category = Configuration)
	FDirectoryPath ExportDirectory;
	
	UPROPERTY(EditAnywhere, Config, Category = Configuration)
	FAssetSettings AssetSettings;

	/* Enables experimental/developing features of JsonAsAsset. Features may not work as intended. */
	UPROPERTY(EditAnywhere, Config, Category = Configuration, AdvancedDisplay)
	bool bEnableExperiments;

	/**
	 * Retrieves assets from a locally hosted API and imports them directly into your project.
	 *
	 * Before initiating the Local Fetch, ensure that all configuration settings are properly set.
	 * For further instructions, please refer to the README.md file found on GitHub.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Local Fetch")
	bool bEnableLocalFetch;

	/**
	 * Specific Paks folder of your game
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Local Fetch - Configuration", meta=(EditCondition="bEnableLocalFetch", DisplayName="Directory"))
	FDirectoryPath ArchiveDirectory;

	/**
	 * Unreal Engine version of your game
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Local Fetch - Configuration", meta=(EditCondition="bEnableLocalFetch", DisplayName="Unreal Engine"))
	TEnumAsByte<ECUE4ParseVersion> UnrealVersion = GAME_UE5_LATEST;

	/**
	 * Specifies the file path to your mappings file
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Local Fetch - Configuration", meta=(EditCondition="bEnableLocalFetch", FilePathFilter="usmap", RelativeToGameDir, DisplayName="Mappings File .usmap"))
	FFilePath MappingFilePath;

	/**
	 * Specifies the main encryption key used for decrypting encrypted game files.
	 *
	 * Default value:
	 *   AES Key: 0x0000000000000000000000000000000000000000000000000000000000000000
	 *
	 * Note: This key is optional for most Unreal Engine games. Override it only if your build uses encryption.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Local Fetch - Encryption", meta=(EditCondition="bEnableLocalFetch", DisplayName="Encryption Key"))
	FString ArchiveKey = "0x0000000000000000000000000000000000000000000000000000000000000000";

	/**
	 * Specifies additional encryption keys used for decrypting encrypted game files.
	 *
	 * Note: These keys are optional for most Unreal Engine games. Override them only if your build uses encryption.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Local Fetch - Encryption", meta=(EditCondition="bEnableLocalFetch", DisplayName="Dynamic Keys"))
	TArray<FLocalFetchAES> DynamicKeys;
	
	/**
	 * Local Fetch URL
	 *
	 * Default: "http://localhost:1500"
	 * Note: This setting is intended for advanced users. It is recommended to leave it unchanged unless you are certain of your requirements.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Local Fetch", meta=(EditCondition="bEnableLocalFetch", DisplayName = "Local Fetch URL"), AdvancedDisplay)
	FString LocalFetchUrl = "http://localhost:1500";
};
