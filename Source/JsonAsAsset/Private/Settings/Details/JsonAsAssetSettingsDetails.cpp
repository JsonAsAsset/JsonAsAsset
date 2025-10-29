/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Settings/Details/JsonAsAssetSettingsDetails.h"
#include "Settings/JsonAsAssetSettings.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Utilities/EngineUtilities.h"

#define LOCTEXT_NAMESPACE "JsonAsAssetSettingsDetails"

void FJsonAsAssetSettingsDetails::ReadConfiguration() {
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
