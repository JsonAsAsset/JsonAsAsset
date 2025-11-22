/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Modules/CloudModule.h"

#include "Modules/UI/StyleModule.h"
#include "Settings/JsonAsAssetSettings.h"
#include "Utilities/EngineUtilities.h"

#ifdef _MSC_VER
#undef GetObject
#endif

bool CloudModule::VerifyActivity(const UJsonAsAssetSettings* Settings) {
	if (Settings->bEnableCloudServer && !IsRunning()) {
		FNotificationInfo Info(FText::FromString("No Cloud Servers are active"));
		
		SetNotificationSubText(Info, FText::FromString(
			"Read documentation on how to start one."
		));

		Info.HyperlinkText = FText::FromString("Learn how to setup");
		Info.Hyperlink = FSimpleDelegate::CreateStatic([]() {
			const FString URL = "https://github.com/JsonAsAsset/JsonAsAsset?tab=readme-ov-file#cloud";
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

	if (!IsReady()) {
		FNotificationInfo Info(FText::FromString("No Cloud Server is initialized"));
		
		SetNotificationSubText(Info, FText::FromString(
			"Read documentation on how to start a profile."
		));

		Info.HyperlinkText = FText::FromString("Learn how to setup");
		Info.Hyperlink = FSimpleDelegate::CreateStatic([]() {
			const FString URL = "https://github.com/JsonAsAsset/JsonAsAsset?tab=readme-ov-file#cloud";
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

bool CloudModule::IsRunning() {
	return IsProcessRunning("j0.dev.exe");
}

void CloudModule::RetrieveMetadata() {
	UJsonAsAssetSettings* MutableSettings = GetMutableDefault<UJsonAsAssetSettings>();
	
	if (MutableSettings->bEnableCloudServer) {
		const auto MetadataResponse = Cloud::Get("/api/metadata");
		if (!MetadataResponse.IsValid()) return;
		
		if (MetadataResponse->HasField(TEXT("name"))) {
			FString Name = MetadataResponse->GetStringField(TEXT("name"));
			MutableSettings->AssetSettings.GameName = Name;
		}

		if (MetadataResponse->HasField(TEXT("major_version"))) {
			const int MajorVersion = MetadataResponse->GetIntegerField(TEXT("major_version"));
				
			MutableSettings->AssetSettings.bUE5Target = MajorVersion == 5;
		}

		SavePluginConfig(MutableSettings);
	}
}

bool CloudModule::IsReady() {
	const auto StatusResponse = Cloud::Get("/api/status");

	return StatusResponse.IsValid();
}