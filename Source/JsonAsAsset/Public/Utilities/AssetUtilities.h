/* Copyright JsonAsAsset Contributors 2024-2026 */

#pragma once

struct JSONASASSET_API FAssetUtilities {
public:
	/* Creates a UPackage to create assets in the Content Browser. */
	static UPackage* CreateAssetPackage(const FString& FullPath);
	static UPackage* CreateAssetPackage(const FString& Name, const FString& OutputPath);
	static UPackage* CreateAssetPackage(const FString& Name, const FString& OutputPath, FString& FailureReason);
	
public:
	/* Importing assets from Cloud */
	template <class T = UObject>
	static void ConstructAssetAsync(const FString& Path, const FString& RealPath, const FString& Type, TFunction<void(TObjectPtr<T>, bool)> OnComplete);
	
	static void CreateTexture(const FString& Path, const FString& FetchPath, const TFunction<void(UTexture*, bool)>& OnComplete);
	static void Fast_CreateTexture(const TSharedPtr<FJsonObject>& JsonExport, const FString& Path, const FString& Type, TArray<uint8> Data, TFunction<void(UTexture*, bool)> OnComplete);
};
