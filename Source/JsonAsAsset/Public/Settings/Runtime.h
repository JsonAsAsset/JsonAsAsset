/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

struct FJCloudProfile {
	FString Name;
};

struct FJRuntime {
	/* If the assets being imported, are from UE5 */
	bool bUE5Target = false;

	/* UE4.22 ~~> 22 */
	int MinorVersion = -1;

	/* UE4.22 ~~> 4 */
	int MajorVersion = -1;

	FJCloudProfile Profile;

	FDirectoryPath ExportDirectory;

	/* Helper Functions ~~~~~~~~~~~ */
	bool IsOlderUE4Target() const;

	/* Update Functions ~~~~~~~~~~~ */
	void Update();
};

extern FJRuntime GJsonAsAssetRuntime;