/* Copyright JsonAsAsset Contributors 2024-2026 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

struct FJCloudProfile {
	FString Name;
};

struct FJRuntime {
	/* UE4.22 ~~> 22 */
	int MinorVersion = -1;

	/* UE4.22 ~~> 4 */
	int MajorVersion = -1;

	FJCloudProfile Profile;
	FDirectoryPath ExportDirectory;

	bool bEnableToolbarToggling;

	/* Helper Functions ~~~~~~~~~~~ */
	bool IsOlderUE4Target() const;
	bool IsUE5() const;
	bool IsUE4() const;

	/* Update Functions ~~~~~~~~~~~ */
	void Update();
};

extern FJRuntime GJsonAsAssetRuntime;