/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "JsonAsAsset.h"
#include "Modules/Cloud/Cloud.h"
#include "Modules/UI/StyleModule.h"
#include "Utilities/EngineUtilities.h"

#if PLATFORM_WINDOWS
static TWeakPtr<SNotificationItem> CloudNotification;
#endif

bool Cloud::Status::IsOpened() {
	return IsProcessRunning("j0.dev.exe");
}

bool Cloud::Status::IsReady() {
	return Get("/api/status").IsValid();
}

bool Cloud::Status::Check(const UJsonAsAssetSettings* Settings) {
	RemoveNotification(CloudNotification);
	
	if (Settings->bEnableCloudServer && !IsOpened()) {
		FNotificationInfo Info(FText::FromString("No Cloud Servers are active"));
		
		SetNotificationSubText(Info, FText::FromString(
			"Read documentation on how to start one."
		));

		Info.HyperlinkText = FText::FromString("Learn how to setup");
		Info.Hyperlink = FSimpleDelegate::CreateStatic([]() {
			LaunchURL(GitHub::README::Cloud);
		});

		Info.bFireAndForget = false;
		Info.FadeOutDuration = 3.0f;
		Info.ExpireDuration = 3.0f;
		Info.bUseLargeFont = false;
		Info.bUseThrobber = false;
		Info.Image = FJsonAsAssetStyle::Get().GetBrush("Toolbar.Icon");

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
			LaunchURL(GitHub::README::Cloud);
		});

		Info.bFireAndForget = false;
		Info.FadeOutDuration = 3.0f;
		Info.ExpireDuration = 3.0f;
		Info.bUseLargeFont = false;
		Info.bUseThrobber = false;
		Info.Image = FJsonAsAssetStyle::Get().GetBrush("Toolbar.Icon");

		CloudNotification = FSlateNotificationManager::Get().AddNotification(Info);
		CloudNotification.Pin()->SetCompletionState(SNotificationItem::CS_Pending);

		return false;
	}

	return true;
}
