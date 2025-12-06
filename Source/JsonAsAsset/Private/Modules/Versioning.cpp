/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Modules/Versioning.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/Log.h"
#include "Modules/UI/StyleModule.h"
#include "Serialization/JsonSerializer.h"
#include "Utilities/EngineUtilities.h"

FJsonAsAssetVersioning GJsonAsAssetVersioning;

void FJsonAsAssetVersioning::SetValid(const bool bValid) {
	bIsValid = bValid;
}

void FJsonAsAssetVersioning::Reset(const int InVersion, const int InLatestVersion, const FString& InHTMLUrl, const FString& InVersionName, const FString& InCurrentVersionName) {
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

void FJsonAsAssetVersioning::Update() {
	if (IsRunningCommandlet() || IsRunningDedicatedServer()) {
		return;
	}
	
	SetValid(false);

	FHttpModule* HttpModule = &FHttpModule::Get();

	const auto Request = HttpModule->CreateRequest();

	Request->SetURL(TEXT("https://api.github.com/repos/JsonAsAsset/JsonAsAsset/releases/latest"));
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("User-Agent"), GJsonAsAssetName.ToString());

	Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Req, const FHttpResponsePtr& Resp, const bool bSuccess) {
		/* Check if the request was successful and the response is valid */
		if (!bSuccess || !Resp.IsValid()) {
			UE_LOG(LogJsonAsAsset, Warning, TEXT("HTTP request failed or no internet connection."));
        		
			return;
		}

		const FString ResponseString = Resp->GetContentAsString();

		/* Deserialize the JSON response */
		TSharedPtr<FJsonObject> JsonObject;
		if (!DeserializeJSONObject(ResponseString, JsonObject)) return;

		/* It must have the name property */
		if (!JsonObject->HasField(TEXT("name"))) {
			return;
		}

		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(GJsonAsAssetName.ToString());

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
				LaunchURL(CapturedUrl);
			});

			Info.bFireAndForget = true;
			Info.FadeOutDuration = 0.1f;
			Info.ExpireDuration = 13.5f;
			Info.bUseLargeFont = false;
			Info.bUseThrobber = false;
			Info.Image = FJsonAsAssetStyle::Get().GetBrush("Toolbar.Icon");

			FSlateNotificationManager::Get().AddNotification(Info);
			IsNotificationShown = true;
		}
	});
	
	const auto Response = FRemoteUtilities::ExecuteRequestSync(Request);

    Request->ProcessRequest();
}
