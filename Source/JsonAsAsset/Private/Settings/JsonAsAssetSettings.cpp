/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Settings/JsonAsAssetSettings.h"

#include "Utilities/EngineUtilities.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "JsonAsAsset"

UJsonAsAssetSettings::UJsonAsAssetSettings():
	/* Default initializers */
	bEnableExperiments(false),
	bEnableCloudServer(false),
	bCustomCloudServer(false)
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("JsonAsAsset");
}

FText UJsonAsAssetSettings::GetSectionText() const {
	return LOCTEXT("SettingsDisplayName", "JsonAsAsset");
}

bool UJsonAsAssetSettings::EnsureExportDirectoryIsValid(UJsonAsAssetSettings* Settings) {
	const FString ExportDirectoryPath = Settings->ExportDirectory.Path;

	if (ExportDirectoryPath.IsEmpty()) {
		return false;
	}

	/* Invalid Export Directory */
	if (ExportDirectoryPath.Contains("\\")) {
		/* Fix up export directory */
		Settings->ExportDirectory.Path = ExportDirectoryPath.Replace(TEXT("\\"), TEXT("/"));
	
		SavePluginConfig(Settings);
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
