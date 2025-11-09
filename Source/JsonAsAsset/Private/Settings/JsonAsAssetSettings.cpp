/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Settings/JsonAsAssetSettings.h"

#include "Utilities/EngineUtilities.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "JsonAsAsset"

UJsonAsAssetSettings::UJsonAsAssetSettings():
	bEnableExperiments(false),
	bEnableCloudServer(false)
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
		ReadAppData();
		
		if (ExportDirectoryPath.IsEmpty()) {
			return false;
		}
	}

	/* Invalid Export Directory */
	if (ExportDirectoryPath.Contains("\\")) {
		/* Fix up export directory */
		Settings->ExportDirectory.Path = ExportDirectoryPath.Replace(TEXT("\\"), TEXT("/"));
	
		SavePluginConfig(Settings);
	}

	return true;
}

bool UJsonAsAssetSettings::IsSetup(UJsonAsAssetSettings* Settings, TArray<FString>& Reasons) {
	const bool IsExportDirectoryValid = EnsureExportDirectoryIsValid(Settings);
	
	if (!IsExportDirectoryValid) {
		Reasons.Add("Export Directory is missing");
	}

	return IsExportDirectoryValid;
}

bool UJsonAsAssetSettings::IsSetup(UJsonAsAssetSettings* Settings) {
	if (Settings == nullptr) return false;
	
	TArray<FString> Params;
	return IsSetup(Settings, Params);
}

void UJsonAsAssetSettings::ReadAppData() {
	UJsonAsAssetSettings* PluginSettings = GetMutableDefault<UJsonAsAssetSettings>();

	/* Get the path to AppData\Roaming */
	FString AppDataPath = FPlatformMisc::GetEnvironmentVariable(TEXT("APPDATA"));
	AppDataPath = FPaths::Combine(AppDataPath, TEXT("FModel/AppSettings.json"));

	FString JsonContent;
        	
	if (FFileHelper::LoadFileToString(JsonContent, *AppDataPath)) {
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
		TSharedPtr<FJsonObject> JsonObject;

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid()) {
			/* Load the PropertiesDirectory and GameDirectory */
			PluginSettings->ExportDirectory.Path = JsonObject->GetStringField(TEXT("PropertiesDirectory")).Replace(TEXT("\\"), TEXT("/"));
		}
	}

	SavePluginConfig(PluginSettings);
}

#undef LOCTEXT_NAMESPACE
