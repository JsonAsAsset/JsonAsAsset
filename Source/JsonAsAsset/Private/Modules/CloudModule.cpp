/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Modules/CloudModule.h"

#include "Modules/UI/StyleModule.h"
#include "Settings/JsonAsAssetSettings.h"
#include "Utilities/EngineUtilities.h"

#ifdef _MSC_VER
#undef GetObject
#endif

bool CloudModule::TryLaunchingCloud(const UJsonAsAssetSettings* Settings) {
	if (Settings->bEnableCloudServer && !Settings->bCustomCloudServer && !IsCloudRunning()) {
		FNotificationInfo Info(FText::FromString("No Cloud Servers are active"));
		
		SetNotificationSubText(Info, FText::FromString(
			"Read documentation on how to start one."
		));

		Info.HyperlinkText = FText::FromString("Documentation");
		Info.Hyperlink = FSimpleDelegate::CreateStatic([]() {
			const FString URL = "https://github.com/JsonAsAsset/JsonAsAsset?tab=readme-ov-file#4-cloud-server";
			FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
		});

		Info.bFireAndForget = false;
		Info.FadeOutDuration = 3.0f;
		Info.ExpireDuration = 3.0f;
		Info.bUseLargeFont = false;
		Info.bUseThrobber = false;
		Info.Image = FJsonAsAssetStyle::Get().GetBrush("JsonAsAsset.Toolbar.Icon");

		RemoveNotification(CloudNotification);

		CloudNotification = FSlateNotificationManager::Get().AddNotification(Info);
		CloudNotification.Pin()->SetCompletionState(SNotificationItem::CS_Pending);

		return false;
	}

	RemoveNotification(CloudNotification);

	return true;
}

bool CloudModule::IsCloudRunning()
{
	return IsProcessRunning("j0.dev.exe");
}

void CloudModule::EnsureGameName(const UJsonAsAssetSettings* Settings) {
	if (Settings->bEnableCloudServer) {
		if (Settings->AssetSettings.GameName.IsEmpty()) {
			const auto MetadataResponse = RequestObjectURL("http://localhost:1500/api/metadata");
			if (!MetadataResponse->HasField("name")) return;
			
			FString Name = MetadataResponse->GetStringField("name");
			UJsonAsAssetSettings* MutableSettings = GetMutableDefault<UJsonAsAssetSettings>();

			MutableSettings->AssetSettings.GameName = Name;
			SavePluginConfig(MutableSettings);
		}
	}
}

bool CloudModule::IsSetup(const UJsonAsAssetSettings* Settings, TArray<FString>& Reasons) {
	return true;
}

bool CloudModule::IsSetup(const UJsonAsAssetSettings* Settings) {
	TArray<FString> Params;
	return IsSetup(Settings, Params);
}
