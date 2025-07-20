/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Utilities/Compatibility.h"
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
	 * This option is useful when you need to re-import textures that have been updated in a newer version of your Cloud build.
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

	/* Game's Project Name (Set by Cloud) */
	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	FString GameName;

	UPROPERTY(EditAnywhere, Config, Category = AssetSettings, meta = (DisplayName = "Save Assets On Import"))
	bool bSavePackagesOnImport;
	
	UPROPERTY(EditAnywhere, Config, Category = AssetSettings)
	TArray<FJPathRedirector> PathRedirectors;
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
	 * Retrieves assets from a Web API and imports references directly into your project.
	 *
	 * For further instructions, please refer to the README.md file found on GitHub.
	 */
	UPROPERTY(EditAnywhere, Config, Category = CloudServer)
	bool bEnableCloudServer;

	/**
	 * DO NOT MODIFY UNLESS YOU KNOW WHAT YOU'RE DOING.
	 *
	 * URL used to fetch from the Cloud.
	 */
	UPROPERTY(EditAnywhere, Config, Category = CloudServer, meta=(EditCondition="bCustomCloudServer", DisplayName = "Cloud Server URL"), AdvancedDisplay)
	FString CloudURL = "http://localhost:1500";

	UPROPERTY(EditAnywhere, Category = CloudServer, meta=(PinHiddenByDefault, InlineEditConditionToggle))
	uint8 bCustomCloudServer : 1;
	
	static bool EnsureExportDirectoryIsValid(UJsonAsAssetSettings* Settings) {
		const FString ExportDirectoryPath = Settings->ExportDirectory.Path;
	
		if (ExportDirectoryPath.IsEmpty()) {
			return false;
		}

		/* Invalid Export Directory */
		if (ExportDirectoryPath.Contains("\\")) {
			/* Fix up export directory */
			Settings->ExportDirectory.Path = ExportDirectoryPath.Replace(TEXT("\\"), TEXT("/"));
		
			Settings->SaveConfig();
	
#if ENGINE_UE5
			Settings->TryUpdateDefaultConfigFile();
			Settings->ReloadConfig(nullptr, nullptr, UE::LCPF_PropagateToInstances);
#else
			Settings->UpdateDefaultConfigFile();
			Settings->ReloadConfig(nullptr, nullptr, UE4::LCPF_PropagateToInstances);
#endif
        	
			Settings->LoadConfig();
		}
	
		return true;
	}

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