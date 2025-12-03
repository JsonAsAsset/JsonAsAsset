/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Runtime.generated.h"

USTRUCT()
struct FJRuntime
{
	GENERATED_BODY()
public:
	/* If the assets being imported, are from UE5 */
	bool bUE5Target = false;

	/* UE4.22 ~~> 22 */
	int MinorVersion = -1;

	/* UE4.22 ~~> 4 */
	int MajorVersion = -1;

	UPROPERTY(Config)
	FDirectoryPath ExportDirectory;
};