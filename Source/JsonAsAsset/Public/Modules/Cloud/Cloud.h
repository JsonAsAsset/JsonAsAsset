/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Serialization/JsonSerializer.h"
#include "Settings/JsonAsAssetSettings.h"

class JSONASASSET_API Cloud {
public:
	static inline FString URL = TEXT("http://localhost:1500");
	static inline FHttpModule* HttpModule = &FHttpModule::Get();

	/* Status ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
public:
	class JSONASASSET_API Status {
	public:
		/* If the Cloud is opened (not if it's ready) */
		static bool IsOpened();

		/* If the Cloud is ready for requests */
		static bool IsReady();

		/* If the app is not ready or not opened, show the user a notification */
		static bool Check(const UJsonAsAssetSettings* Settings);
	};

public:
	static void Update();
	
	/* Export Endpoints ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
public:
	static inline FString ExportURL = TEXT("/api/export");

	class JSONASASSET_API Export {
	public:
		class JSONASASSET_API Array {
		public:
			static TArray<TSharedPtr<FJsonValue>> Get(const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {});
			static TArray<TSharedPtr<FJsonValue>> Get(const FString& Path, const bool Raw, TMap<FString, FString> Parameters = {}, const TMap<FString, FString>& Headers = {});
			static TArray<TSharedPtr<FJsonValue>> GetRaw(const FString& Path, const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {});
		};
		
		static TSharedPtr<FJsonObject> Get(const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {});
		static TSharedPtr<FJsonObject> Get(const FString& Path, const bool Raw, TMap<FString, FString> Parameters = {}, const TMap<FString, FString>& Headers = {});
		static TSharedPtr<FJsonObject> GetRaw(const FString& Path, const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {});
	};

public:
	static auto SendRequest(const FString& RequestURL, const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {});
	static TSharedPtr<FJsonObject> Get(const FString& RequestURL, const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {});
	static TArray<uint8> GetRaw(const FString& RequestURL, const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {});
	static TArray<TSharedPtr<FJsonValue>> GetExports(const FString& RequestURL, const TMap<FString, FString>& Parameters = {});
};