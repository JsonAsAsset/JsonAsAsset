/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IPluginManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UI/StyleModule.h"
#include "Utilities/EngineUtilities.h"

struct FJsonAsAssetVersioning {
	bool bNewVersionAvailable = false;
	bool bFutureVersion = false;
	bool bLatestVersion = false;
    
	FJsonAsAssetVersioning() = default;
    
	FJsonAsAssetVersioning(const int Version, const int LatestVersion, const FString& InHTMLUrl, const FString& VersionName, const FString& CurrentVersionName)
		: Version(Version)
		, LatestVersion(LatestVersion)
		, VersionName(VersionName)
		, CurrentVersionName(CurrentVersionName)
		, HTMLUrl(InHTMLUrl)
	{
		bNewVersionAvailable = LatestVersion > Version;
		bFutureVersion = Version > LatestVersion;
        
		bLatestVersion = !(bNewVersionAvailable || bFutureVersion);
	}

	int Version = 0;
	int LatestVersion = 0;

	FString VersionName = "";
	FString CurrentVersionName = "";

	FString HTMLUrl = "";

	bool bIsValid = false;

	void SetValid(const bool bValid) {
		bIsValid = bValid;
	}

	void Reset(const int InVersion, const int InLatestVersion, const FString& InHTMLUrl, const FString& InVersionName, const FString& InCurrentVersionName);

	void Update();
};

extern FJsonAsAssetVersioning GJsonAsAssetVersioning;