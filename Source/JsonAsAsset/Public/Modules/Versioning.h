/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"

/* GitHub Versioning Updates for JsonAsAsset */
struct FJsonAsAssetVersioning {
	/* Constructors ~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	FJsonAsAssetVersioning() = default;
	FJsonAsAssetVersioning(const int Version, const int LatestVersion, const FString& InHTMLUrl, const FString& VersionName, const FString& CurrentVersionName)
		: Version(Version)
		, VersionName(VersionName)
		, CurrentVersionName(CurrentVersionName)
		, HTMLUrl(InHTMLUrl)
		, LatestVersion(LatestVersion)
	{
		bNewVersionAvailable = LatestVersion > Version;
		bFutureVersion = Version > LatestVersion;
        
		bLatestVersion = !(bNewVersionAvailable || bFutureVersion);
	}

	/* Functions ~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	void SetValid(const bool bValid);
	void Reset(const int InVersion, const int InLatestVersion, const FString& InHTMLUrl, const FString& InVersionName, const FString& InCurrentVersionName);
	void Update();
	
	/* Static Helper Variables ~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	bool bNewVersionAvailable = false;
	bool bFutureVersion = false;
	bool bLatestVersion = false;

	int Version = 0;
	
	FString VersionName = "";
	FString CurrentVersionName = "";
	FString HTMLUrl = "";

	bool bIsValid = false;

	/* .uplugin Version */
	int LatestVersion = 0;
};

extern FJsonAsAssetVersioning GJsonAsAssetVersioning;