/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Settings/Details/JsonAsAssetSettingsDetails.h"
#include "Settings/JsonAsAssetSettings.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Utilities/RemoteUtilities.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Modules/LocalFetchModule.h"
#include "Utilities/EngineUtilities.h"

#if ENGINE_UE4
#include "DetailCategoryBuilder.h"
#endif

#define LOCTEXT_NAMESPACE "JsonAsAssetSettingsDetails"

TSharedRef<IDetailCustomization> FJsonAsAssetSettingsDetails::MakeInstance() {
	return MakeShareable(new FJsonAsAssetSettingsDetails);
}

void FJsonAsAssetSettingsDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	EditConfiguration(DetailBuilder);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void FJsonAsAssetSettingsDetails::EditConfiguration(IDetailLayoutBuilder& DetailBuilder) {
	IDetailCategoryBuilder& AssetCategory = DetailBuilder.EditCategory("Configuration", FText::GetEmpty(), ECategoryPriority::Important);
	
	AssetCategory.AddCustomRow(LOCTEXT("UseFModelAppSettings", "UseFModelAppSettings"))
    .NameContent()
    [
        /* This defines the name/title of the row */
        SNew(STextBlock)
        .Text(LOCTEXT("UseFModelAppSettings", "Load External Configuration"))
        .Font(IDetailLayoutBuilder::GetDetailFont())
    ]
    .ValueContent()
    [
        /* Now we define the value/content of the row */
        SNew(SButton)
        .Text(LOCTEXT("UseFModelAppSettings_Text", "FModel Settings"))
    	.ToolTipText(LOCTEXT("UseFModelAppSettings_Tooltip", "Imports settings from AppData/Roaming/FModel/AppSettings.json"))
        .OnClicked_Lambda([this]()
        {
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
                    PluginSettings->ArchiveDirectory.Path = JsonObject->GetStringField(TEXT("GameDirectory")).Replace(TEXT("\\"), TEXT("/"));

					const FString GameDirectory = JsonObject->GetStringField(TEXT("GameDirectory"));

                    /* Handling AES Keys */
                    if (JsonObject->HasField(TEXT("PerDirectory"))) {
                        const TSharedPtr<FJsonObject> PerDirectoryObject = JsonObject->GetObjectField(TEXT("PerDirectory"));

                        if (PerDirectoryObject->HasField(GameDirectory)) {
                            const TSharedPtr<FJsonObject> PakSettings = PerDirectoryObject->GetObjectField(GameDirectory);

                            if (PakSettings->HasField(TEXT("AesKeys"))) {
                                const TSharedPtr<FJsonObject> AesKeysObject = PakSettings->GetObjectField(TEXT("AesKeys"));

                                if (AesKeysObject->HasField(TEXT("mainKey"))) {
                                    PluginSettings->ArchiveKey = AesKeysObject->GetStringField(TEXT("mainKey"));
                                }

                                if (AesKeysObject->HasField(TEXT("dynamicKeys"))) {
                                    const TArray<TSharedPtr<FJsonValue>> DynamicKeysArray = AesKeysObject->GetArrayField(TEXT("dynamicKeys"));
                                    PluginSettings->DynamicKeys.Empty();

                                    for (const auto& KeyValue : DynamicKeysArray) {
                                        const TSharedPtr<FJsonObject> KeyObject = KeyValue->AsObject();

                                        if (KeyObject.IsValid()) {
                                            FLocalFetchAES NewKey;
                                        	
                                            NewKey.Guid = KeyObject->GetStringField(TEXT("guid"));
                                            NewKey.Value = KeyObject->GetStringField(TEXT("key"));
                                        	
                                            PluginSettings->DynamicKeys.Add(NewKey);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

        	SavePluginConfig(PluginSettings);

            return FReply::Handled();
        })
    ];
}

#undef LOCTEXT_NAMESPACE
