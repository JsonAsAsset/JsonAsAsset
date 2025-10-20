/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS
	static TWeakPtr<SNotificationItem> CloudNotification;
#endif

class UJsonAsAssetSettings;

class CloudModule {
public:
	static bool VerifyActivity(const UJsonAsAssetSettings* Settings);
	static bool IsRunning();

	/* If GameName isn't set, try getting it from the API */
	static void EnsureGameName(const UJsonAsAssetSettings* Settings);
};