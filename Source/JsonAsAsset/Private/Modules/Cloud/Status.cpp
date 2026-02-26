/* Copyright JsonAsAsset Contributors 2024-2026 */

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

void Cloud::Status::IsReady(TFunction<void(bool)> OnResponse) {
	Get("/api/status", {}, {},
		[OnResponse](const TSharedPtr<FJsonObject>& Json) {
			OnResponse(Json.IsValid());
		}
	);
}

void Cloud::Status::Check(const UJsonAsAssetSettings* Settings,TFunction<void(bool)> OnResponse) {
	RemoveNotification(CloudNotification);
	
	if (Settings->EnableCloudServer && !IsOpened()) {
		CloudNotification = AppendNotificationWithHandler(
			FText::FromString("No Active Cloud Instance"),
			FText::FromString("Read documentation on how to start one."),
			0.5f,
			FJsonAsAssetStyle::Get().GetBrush("Toolbar.Icon"),
			SNotificationItem::CS_None,
			false,
			0.0f,
			[](FNotificationInfo& Info) {
				Info.HyperlinkText = FText::FromString("Learn how to setup");
				Info.Hyperlink = FSimpleDelegate::CreateStatic([]() {
					LaunchURL(GitHub::README::Cloud);
				});
			}
		);
		
		OnResponse(false);
		
		return;
	}

	IsReady([OnResponse](const bool bReady) {
		OnResponse(bReady);
	});
}

bool Cloud::Status::ShouldWaitUntilInitialized(const UJsonAsAssetSettings* Settings) {
	return Settings->EnableCloudServer && IsOpened();
}
