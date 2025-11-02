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

	void Reset(const int InVersion, const int InLatestVersion, const FString& InHTMLUrl, const FString& InVersionName, const FString& InCurrentVersionName) {
		Version = InVersion;
		LatestVersion = InLatestVersion;
		VersionName = InVersionName;
		CurrentVersionName = InCurrentVersionName;
		HTMLUrl = InHTMLUrl;

		bNewVersionAvailable = LatestVersion > Version;
		bFutureVersion = Version > LatestVersion;
		bLatestVersion = !(bNewVersionAvailable || bFutureVersion);
		SetValid(true);
	}

	void Update() {
		if (IsRunningCommandlet() || IsRunningDedicatedServer()) {
			return;
		}
		
		SetValid(false);

		FHttpModule* HttpModule = &FHttpModule::Get();

	#if ENGINE_UE5
		const TSharedRef<IHttpRequest> Request = HttpModule->CreateRequest();
	#else
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule->CreateRequest();
	#endif

		Request->SetURL(TEXT("https://api.github.com/repos/JsonAsAsset/JsonAsAsset/releases/latest"));
		Request->SetVerb(TEXT("GET"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		Request->SetHeader(TEXT("User-Agent"), TEXT("JsonAsAsset"));

		Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Req, const FHttpResponsePtr& Resp, const bool bSuccess) {
			/* Check if the request was successful and the response is valid */
			if (!bSuccess || !Resp.IsValid()) {
				UE_LOG(LogJsonAsAsset, Warning, TEXT("HTTP request failed or no internet connection."));
        			
				return;
			}

			const FString ResponseString = Resp->GetContentAsString();

			/* Deserialize the JSON response */
			TSharedPtr<FJsonObject> JsonObject;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
			if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid()) {
				return;
			}

			/* It must have the name property */
			if (!JsonObject->HasField(TEXT("name"))) {
				return;
			}

			const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("JsonAsAsset");

			const FString VersionName = JsonObject->GetStringField(TEXT("name"));
			const FString CurrentVersionName = Plugin->GetDescriptor().VersionName;

			const int LatestVersion = ConvertVersionStringToInt(VersionName);
			const int CurrentVersion = ConvertVersionStringToInt(Plugin->GetDescriptor().VersionName);

			Reset(CurrentVersion, LatestVersion, JsonObject->GetStringField(TEXT("html_url")), VersionName, CurrentVersionName);

			static bool IsNotificationShown = false;
			
			if (bNewVersionAvailable && !IsNotificationShown) {
				const FString CapturedUrl = HTMLUrl;

				FNotificationInfo Info(FText::FromString(VersionName + " is now available!"));
		
				SetNotificationSubText(Info, FText::FromString(
					"Get the latest features and improvements in the new version."
				));

				Info.HyperlinkText = FText::FromString("GitHub Release");
				Info.Hyperlink = FSimpleDelegate::CreateLambda([CapturedUrl]() {
					FPlatformProcess::LaunchURL(*CapturedUrl, nullptr, nullptr);
				});

				Info.bFireAndForget = true;
				Info.FadeOutDuration = 0.1f;
				Info.ExpireDuration = 13.5f;
				Info.bUseLargeFont = false;
				Info.bUseThrobber = false;
				Info.Image = FJsonAsAssetStyle::Get().GetBrush("JsonAsAsset.Toolbar.Icon");

				FSlateNotificationManager::Get().AddNotification(Info);
				IsNotificationShown = true;
			}
		});
		
	#if ENGINE_UE5
		const TSharedPtr<IHttpResponse> Response = FRemoteUtilities::ExecuteRequestSync(Request);
	#else
		const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response = FRemoteUtilities::ExecuteRequestSync(Request);
	#endif

	    Request->ProcessRequest();
	}
};

extern FJsonAsAssetVersioning GJsonAsAssetVersioning;