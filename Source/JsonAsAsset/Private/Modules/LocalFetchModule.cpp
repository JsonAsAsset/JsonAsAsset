/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Modules/LocalFetchModule.h"

#include "Interfaces/IPluginManager.h"
#include "Modules/UI/StyleModule.h"
#include "Settings/JsonAsAssetSettings.h"
#include "Windows/WindowsHWrapper.h"
#include "Utilities/Compatibility.h"
#include "Utilities/EngineUtilities.h"

#ifdef _MSC_VER
#undef GetObject
#endif

bool LocalFetchModule::LaunchLocalFetch() {
	const UJsonAsAssetSettings* Settings = GetMutableDefault<UJsonAsAssetSettings>();

	FString PluginFolder; {
		const TSharedPtr<IPlugin> PluginInfo = IPluginManager::Get().FindPlugin("JsonAsAsset");

		if (PluginInfo.IsValid()) {
			PluginFolder = PluginInfo->GetBaseDir();
		}
	}

	FString FullPath = FPaths::ConvertRelativePathToFull(PluginFolder + "/Dependencies/LocalFetch/Release/Win64/LocalFetch.exe");
	FString Params = "--urls=" + Settings->LocalFetchUrl;

#if ENGINE_UE5
	return FPlatformProcess::LaunchFileInDefaultExternalApplication(*FullPath, *Params, ELaunchVerb::Open);
#else
	FPlatformProcess::LaunchFileInDefaultExternalApplication(*FullPath, *Params, ELaunchVerb::Open);
	
	return IsLocalFetchRunning();
#endif
}

bool LocalFetchModule::TryLaunchingLocalFetch(const UJsonAsAssetSettings* Settings) {
	const bool bIsLocalHost = Settings->LocalFetchUrl.StartsWith("http://localhost");

	if (bIsLocalHost && !IsLocalFetchRunning()) {
		const bool bLocalFetchLaunched = LaunchLocalFetch();

		if (!bLocalFetchLaunched) {
			FNotificationInfo Info(FText::FromString("Local Fetch Failed To Launch"));
			
			SetNotificationSubText(Info, FText::FromString(
				"Missing LocalFetch.exe file. Please clone JsonAsAsset with submodules."
			));

			Info.HyperlinkText = FText::FromString("Documentation");
			Info.Hyperlink = FSimpleDelegate::CreateStatic([]() {
				const FString URL = "https://github.com/JsonAsAsset/JsonAsAsset";
				FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
			});

			Info.bFireAndForget = false;
			Info.FadeOutDuration = 3.0f;
			Info.ExpireDuration = 3.0f;
			Info.bUseLargeFont = false;
			Info.bUseThrobber = false;
			Info.Image = FJsonAsAssetStyle::Get().GetBrush("JsonAsAsset.Toolbar.Icon");

			Info.ButtonDetails.Add(
				FNotificationButtonInfo(FText::FromString("Retry Launching"), FText::GetEmpty(),
					FSimpleDelegate::CreateStatic([]() {
						RemoveNotification(LocalFetchNotification);

						LaunchLocalFetch();
					})
				)
			);

			RemoveNotification(LocalFetchNotification);

			LocalFetchNotification = FSlateNotificationManager::Get().AddNotification(Info);
			LocalFetchNotification.Pin()->SetCompletionState(SNotificationItem::CS_Pending);

			return false;
		}
		
		RemoveNotification(LocalFetchNotification);
	}

	return true;
}

void LocalFetchModule::CloseLocalFetch() {
	CloseApplicationByProcessName("LocalFetch.exe");
}

bool LocalFetchModule::IsLocalFetchRunning()
{
	return IsProcessRunning("LocalFetch.exe") || IsProcessRunning("j0.dev.exe");
}

void LocalFetchModule::EnsureGameName(const UJsonAsAssetSettings* Settings) {
	if (Settings->bEnableLocalFetch) {
		if (Settings->AssetSettings.GameName.IsEmpty()) {
			SendHttpRequest(TEXT("http://localhost:1500/api/name"), [](FHttpRequestPtr, const FHttpResponsePtr& Response, const bool bWasSuccessful) {
				if (bWasSuccessful && Response.IsValid()) {
					UJsonAsAssetSettings* MutableSettings = GetMutableDefault<UJsonAsAssetSettings>();

					MutableSettings->AssetSettings.GameName = Response->GetContentAsString();
					SavePluginConfig(MutableSettings);
				}
			});
		}
	}
}

bool LocalFetchModule::IsSetup(const UJsonAsAssetSettings* Settings, TArray<FString>& Reasons) {
	if (!Settings->bEnableLocalFetch) {
		return true;
	}
	
	if (Settings->MappingFilePath.FilePath.IsEmpty()) {
		Reasons.Add("Mappings file is missing");
	}

	if (Settings->ArchiveDirectory.Path.IsEmpty()) {
		Reasons.Add("Archive directory is missing");
	}

	return !(Settings->MappingFilePath.FilePath.IsEmpty() || Settings->ArchiveDirectory.Path.IsEmpty());
}

bool LocalFetchModule::IsSetup(const UJsonAsAssetSettings* Settings) {
	TArray<FString> Params;
	return IsSetup(Settings, Params);
}
