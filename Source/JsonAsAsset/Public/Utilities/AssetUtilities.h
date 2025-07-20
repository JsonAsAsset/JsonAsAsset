/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Compatibility.h"

class JSONASASSET_API FAssetUtilities {
public:
	/*
	* Creates a UPackage to create assets in the Content Browser.
	* 
	* @return Package
	*/
	static UPackage* CreateAssetPackage(const FString& FullPath);
	static UPackage* CreateAssetPackage(const FString& Name, const FString& OutputPath);
	static UPackage* CreateAssetPackage(const FString& Name, const FString& OutputPath, UPackage*& OutOutermostPkg);
	
public:
	/* Importing assets from Cloud */
	template <class T = UObject>
	static bool ConstructAsset(const FString& Path, const FString& Type, TObjectPtr<T>& OutObject, bool& bSuccess);
	
	static bool Construct_TypeTexture(const FString& Path, const FString& FetchPath, UTexture*& OutTexture);

	static TSharedPtr<FJsonObject> API_RequestExports(const FString& Path, const FString& FetchPath = "/api/export?raw=true&path=");
};
