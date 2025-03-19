﻿/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Interfaces/IMainFrameModule.h"
#include "IContentBrowserSingleton.h"
#include "Windows/WindowsHWrapper.h"
#include "DesktopPlatformModule.h"
#include "ContentBrowserModule.h"
#include "IDesktopPlatform.h"
#include "AssetUtilities.h"
#include "HttpModule.h"
#include "TlHelp32.h"
#include "Json.h"
#include "PluginUtils.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IPluginManager.h"
#include "Settings/JsonAsAssetSettings.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

#if (ENGINE_MAJOR_VERSION != 4 || ENGINE_MINOR_VERSION < 27)
#include "Engine/DeveloperSettings.h"
#endif

inline bool HandlePackageCreation(UObject* Asset, UPackage* Package) {
	FAssetRegistryModule::AssetCreated(Asset);
	if (!Asset->MarkPackageDirty()) return false;
	
	Package->SetDirtyFlag(true);
	Asset->PostEditChange();
	Asset->AddToRoot();
	
	Package->FullyLoad();

	/* Browse to newly added Asset */
	const TArray<FAssetData>& Assets = {Asset};
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().SyncBrowserToAssets(Assets);

	return true;
}

/**
 * Get the asset currently selected in the Content Browser.
 * 
 * @return Selected Asset
 */
template <typename T>
T* GetSelectedAsset(const bool SupressErrors = false) {
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

	if (SelectedAssets.Num() == 0) {
		if (SupressErrors == true) {
			return nullptr;
		}
		
		GLog->Log("JsonAsAsset: [GetSelectedAsset] None selected, returning nullptr.");

		const FText DialogText = FText::Format(
			FText::FromString(TEXT("Importing an asset of type '{0}' requires a base asset selected to modify. Select one in your content browser.")),
			FText::FromString(T::StaticClass()->GetName())
		);

		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		
		return nullptr;
	}

	UObject* SelectedAsset = SelectedAssets[0].GetAsset();
	T* CastedAsset = Cast<T>(SelectedAsset);

	if (!CastedAsset) {
		if (SupressErrors == true) {
			return nullptr;
		}
		
		GLog->Log("JsonAsAsset: [GetSelectedAsset] Selected asset is not of the required class, returning nullptr.");

		const FText DialogText = FText::Format(
			FText::FromString(TEXT("The selected asset is not of type '{0}'. Please select a valid asset.")),
			FText::FromString(T::StaticClass()->GetName())
		);

		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		
		return nullptr;
	}

	return CastedAsset;
}

template <typename TEnum> 
TEnum StringToEnum(const FString& StringValue) {
	return StaticEnum<TEnum>() ? static_cast<TEnum>(StaticEnum<TEnum>()->GetValueByNameString(StringValue)) : TEnum();
}

inline TSharedPtr<FJsonObject> FindExport(const TSharedPtr<FJsonObject>& Export, const TArray<TSharedPtr<FJsonValue>>& File) {
	FString string_int; Export->GetStringField(TEXT("ObjectPath")).Split(".", nullptr, &string_int);
	
	return File[FCString::Atoi(*string_int)]->AsObject();
}

inline void SpawnPrompt(const FString& Title, const FString& Text) {
	FText DialogTitle = FText::FromString(Title);
	FText DialogMessage = FText::FromString(Text);

	FMessageDialog::Open(EAppMsgType::Ok, DialogMessage);
}

/* Gets all assets in selected folder */
inline TArray<FAssetData> GetAssetsInSelectedFolder() {
	TArray<FAssetData> AssetDataList;

	/* Get the Content Browser Module */
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	TArray<FString> SelectedFolders;
	ContentBrowserModule.Get().GetSelectedPathViewFolders(SelectedFolders);

	if (SelectedFolders.Num() == 0) {
		UE_LOG(LogTemp, Warning, TEXT("No folder selected in the Content Browser."));
		return AssetDataList; 
	}

	FString CurrentFolder = SelectedFolders[0];

	/* Check if the folder is the root folder, and show a prompt if */
	if (CurrentFolder == "/Game") {
		SpawnPrompt(
			"Action Not Allowed in Root Content Folder",
			"You can't do this action in the root folder, this will stall the editor for a long time."
		);
		return AssetDataList;
	}

	/* Get the Asset Registry Module */
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().SearchAllAssets(true);

	/* Get all assets in the folder and its subfolders */
	AssetRegistryModule.Get().GetAssetsByPath(FName(*CurrentFolder), AssetDataList, true);

	return AssetDataList;
}

inline TArray<TSharedPtr<FJsonValue>> RequestExports(const FString& Path) {
	TArray<TSharedPtr<FJsonValue>> Exports = {};

	const TSharedPtr<FJsonObject> Response = FAssetUtilities::API_RequestExports(Path);
	if (Response == nullptr || Path.IsEmpty()) return Exports;

	if (Response->HasField(TEXT("errored"))) {
		UE_LOG(LogJson, Log, TEXT("Error from response \"%s\""), *Path);
		return Exports;
	}

	Exports = Response->GetArrayField(TEXT("jsonOutput"));
	
	return Exports;
}

inline TSharedPtr<FJsonObject> RequestExport(const FString& FetchPath = "/api/export?raw=true&path=", const FString& Path = "") {
	static TMap<FString, TSharedPtr<FJsonObject>> ExportCache;

	if (Path.IsEmpty()) return TSharedPtr<FJsonObject>();

	/* Check cache first */
	if (TSharedPtr<FJsonObject>* CachedResponse = ExportCache.Find(Path)) {
		return *CachedResponse;
	}

	/* Fetch from API */
	TSharedPtr<FJsonObject> Response = FAssetUtilities::API_RequestExports(Path, FetchPath);
	
	if (Response) {
		ExportCache.Add(Path, Response);
	}

	return Response;
}

inline bool IsProcessRunning(const FString& ProcessName) {
	bool bIsRunning = false;

	/* Convert FString to WCHAR */
	const TCHAR* ProcessNameChar = *ProcessName;
	const WCHAR* ProcessNameWChar = (const WCHAR*)ProcessNameChar;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 ProcessEntry;
		ProcessEntry.dwSize = sizeof(ProcessEntry);

		if (Process32First(hSnapshot, &ProcessEntry)) {
			do {
				if (_wcsicmp(ProcessEntry.szExeFile, ProcessNameWChar) == 0) {
					bIsRunning = true;
					break;
				}
			} while (Process32Next(hSnapshot, &ProcessEntry));
		}

		CloseHandle(hSnapshot);
	}

	return bIsRunning;
}

inline TSharedPtr<FJsonObject> GetExport(const FString& Type, TArray<TSharedPtr<FJsonValue>> AllJsonObjects, bool bGetProperties = false) {
	for (TSharedPtr<FJsonValue> Value : AllJsonObjects) {
		const TSharedPtr<FJsonObject> ValueObject = Value->AsObject();

		if (ValueObject->GetStringField(TEXT("Type")) == Type) {
			if (bGetProperties) {
				return ValueObject->GetObjectField(TEXT("Properties"));
			}
			
			return ValueObject;
		}
	}
	
	return nullptr;
}

inline TSharedPtr<FJsonObject> GetExport(const FJsonObject* PackageIndex, TArray<TSharedPtr<FJsonValue>> AllJsonObjects) {
	FString ObjectName = PackageIndex->GetStringField(TEXT("ObjectName")); /* Class'Asset:ExportName' */
	FString ObjectPath = PackageIndex->GetStringField(TEXT("ObjectPath")); /* Path/Asset.Index */
	FString Outer;
	
	/* Clean up ObjectName (Class'Asset:ExportName' --> Asset:ExportName --> ExportName) */
	ObjectName.Split("'", nullptr, &ObjectName);
	ObjectName.Split("'", &ObjectName, nullptr);

	if (ObjectName.Contains(":")) {
		ObjectName.Split(":", nullptr, &ObjectName); /* Asset:ExportName --> ExportName */
	}

	if (ObjectName.Contains(".")) {
		ObjectName.Split(".", nullptr, &ObjectName);
	}

	if (ObjectName.Contains(".")) {
		ObjectName.Split(".", &Outer, &ObjectName);
	}

	int Index = 0;

	/* Search for the object in the AllJsonObjects array */
	for (const TSharedPtr<FJsonValue>& Value : AllJsonObjects) {
		const TSharedPtr<FJsonObject> ValueObject = Value->AsObject();

		FString Name;
		if (ValueObject->TryGetStringField(TEXT("Name"), Name) && Name == ObjectName) {
			if (ValueObject->HasField(TEXT("Outer")) && !Outer.IsEmpty()) {
				FString OuterName = ValueObject->GetStringField(TEXT("Outer"));

				if (OuterName == Outer) {
					return AllJsonObjects[Index]->AsObject();
				}
			} else {
				return ValueObject;
			}
		}

		Index++;
	}

	return nullptr;
}

inline bool IsProperExportData(const TSharedPtr<FJsonObject>& JsonObject) {
	/* Property checks */
	if (!JsonObject.IsValid() ||
		!JsonObject->HasField(TEXT("Type")) ||
		!JsonObject->HasField(TEXT("Name")) ||
		!JsonObject->HasField(TEXT("Properties"))
	) return false;

	return true;
}

inline bool DeserializeJSON(const FString& FilePath, TArray<TSharedPtr<FJsonValue>>& JsonParsed) {
	if (FPaths::FileExists(FilePath)) {
		FString ContentBefore;
		
		if (FFileHelper::LoadFileToString(ContentBefore, *FilePath)) {
			FString Content = FString(TEXT("{\"data\": "));
			Content.Append(ContentBefore);
			Content.Append(FString("}"));

			const TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Content);

			TSharedPtr<FJsonObject> JsonObject;
			if (FJsonSerializer::Deserialize(JsonReader, JsonObject)) {
				JsonParsed = JsonObject->GetArrayField(TEXT("data"));
			
				return true;
			}
		}
	}

	return false;
}

inline TArray<FString> OpenFileDialog(const FString& Title, const FString& Type)
{
	TArray<FString> ReturnValue;

	/* Window Handler for Windows */
	void* ParentWindowHandle = nullptr;
	IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
	TSharedPtr<SWindow> MainWindow = MainFrameModule.GetParentWindow();

	if (MainWindow.IsValid() && MainWindow->GetNativeWindow().IsValid()) {
		ParentWindowHandle = MainWindow->GetNativeWindow()->GetOSWindowHandle();
	}

	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);
	FString DefaultPath = FString("");

	if (!ClipboardContent.IsEmpty()) {
		if (FPaths::FileExists(ClipboardContent)) {
			DefaultPath = FPaths::GetPath(ClipboardContent);
		}
		else if (FPaths::DirectoryExists(ClipboardContent)) {
			DefaultPath = ClipboardContent;
		}
	}

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform) {
		uint32 SelectionFlag = 1;
		DesktopPlatform->OpenFileDialog(ParentWindowHandle, Title, DefaultPath, FString(""), Type, SelectionFlag, ReturnValue);
	}

	return ReturnValue;
}

inline TArray<FString> OpenFolderDialog(const FString& Title) {
	TArray<FString> ReturnValue;

	/* Window Handler for Windows */
	void* ParentWindowHandle = nullptr;

	IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
	TSharedPtr<SWindow> MainWindow = MainFrameModule.GetParentWindow();

	if (MainWindow.IsValid() && MainWindow->GetNativeWindow().IsValid()) {
		ParentWindowHandle = MainWindow->GetNativeWindow()->GetOSWindowHandle();
	}

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform) {
		FString SelectedFolder;

		/* Open Folder Dialog */
		if (DesktopPlatform->OpenDirectoryDialog(ParentWindowHandle, Title, FString(""), SelectedFolder)) {
			ReturnValue.Add(SelectedFolder);
		}
	}

	return ReturnValue;
}

/* Filter to remove */
inline TSharedPtr<FJsonObject> RemovePropertiesShared(const TSharedPtr<FJsonObject>& Input, const TArray<FString>& RemovedProperties) {
	TSharedPtr<FJsonObject> ClonedJsonObject = MakeShareable(new FJsonObject(*Input));
    
	for (const FString& Property : RemovedProperties) {
		if (ClonedJsonObject->HasField(Property)) {
			ClonedJsonObject->RemoveField(Property);
		}
	}
    
	return ClonedJsonObject;
}

/* Filter to whitelist */
inline TSharedPtr<FJsonObject> KeepPropertiesShared(const TSharedPtr<FJsonObject>& Input, TArray<FString> WhitelistProperties) {
	const TSharedPtr<FJsonObject> RawSharedPtrData = MakeShared<FJsonObject>();

	for (const FString& Property : WhitelistProperties) {
		if (Input->HasField(Property)) {
			RawSharedPtrData->SetField(Property, Input->TryGetField(Property));
		}
	}

	return RawSharedPtrData;
}

inline void SavePluginConfig(UDeveloperSettings* EditorSettings) {
	EditorSettings->SaveConfig();
	
#if ENGINE_MAJOR_VERSION >= 5
	EditorSettings->TryUpdateDefaultConfigFile();
	EditorSettings->ReloadConfig(nullptr, nullptr, UE::LCPF_PropagateToInstances);
#else
	EditorSettings->UpdateDefaultConfigFile();
	EditorSettings->ReloadConfig(nullptr, nullptr, UE4::LCPF_PropagateToInstances);
#endif
        	
	EditorSettings->LoadConfig();
}

/* Simple handler for JsonArray */
inline auto ProcessJsonArrayField(const TSharedPtr<FJsonObject>& ObjectField, const FString& ArrayFieldName,
                                  const TFunction<void(const TSharedPtr<FJsonObject>&)>& ProcessObjectFunction) -> void
{
	const TArray<TSharedPtr<FJsonValue>>* JsonArray;
	
	if (ObjectField->TryGetArrayField(ArrayFieldName, JsonArray)) {
		for (const auto& JsonValue : *JsonArray) {
			if (const TSharedPtr<FJsonObject> JsonItem = JsonValue->AsObject()) {
				ProcessObjectFunction(JsonItem);
			}
		}
	}
}

inline auto ProcessExports(const TArray<TSharedPtr<FJsonValue>>& Exports,
						   const TFunction<void(const TSharedPtr<FJsonObject>&)>& ProcessObjectFunction) -> void
{
	for (const auto& Json : Exports) {
		if (const TSharedPtr<FJsonObject> JsonObject = Json->AsObject()) {
			ProcessObjectFunction(JsonObject);
		}
	}
}

/* ReSharper disable once CppParameterNeverUsed */
inline void SetNotificationSubText(FNotificationInfo& Notification, const FText& SubText) {
#if ENGINE_MAJOR_VERSION >= 5
	Notification.SubText = SubText;
#endif
}

/* Show the user a Notification */
inline auto AppendNotification(const FText& Text, const FText& SubText, const float ExpireDuration,
                               const SNotificationItem::ECompletionState CompletionState, const bool bUseSuccessFailIcons,
                               const float WidthOverride) -> void
{
	FNotificationInfo Info = FNotificationInfo(Text);
	Info.ExpireDuration = ExpireDuration;
	Info.bUseLargeFont = true;
	Info.bUseSuccessFailIcons = bUseSuccessFailIcons;
	Info.WidthOverride = FOptionalSize(WidthOverride);
	
	SetNotificationSubText(Info, SubText);

	const TSharedPtr<SNotificationItem> NotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationPtr->SetCompletionState(CompletionState);
}

/* Show the user a Notification with Subtext */
inline auto AppendNotification(const FText& Text, const FText& SubText, float ExpireDuration,
                               const FSlateBrush* SlateBrush, SNotificationItem::ECompletionState CompletionState,
                               const bool bUseSuccessFailIcons, const float WidthOverride) -> void
{
	FNotificationInfo Info = FNotificationInfo(Text);
	Info.ExpireDuration = ExpireDuration;
	Info.bUseLargeFont = true;
	Info.bUseSuccessFailIcons = bUseSuccessFailIcons;
	Info.WidthOverride = FOptionalSize(WidthOverride);
	Info.Image = SlateBrush;

	SetNotificationSubText(Info, SubText);

	const TSharedPtr<SNotificationItem> NotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationPtr->SetCompletionState(CompletionState);
}

inline int32 ConvertVersionStringToInt(const FString& VersionStr) {
	return FCString::Atoi(*VersionStr.Replace(TEXT("."), TEXT("")));
}

inline FString ReadPathFromObject(const TSharedPtr<FJsonObject>* PackageIndex) {
	FString ObjectType, ObjectName, ObjectPath, Outer;
	PackageIndex->Get()->GetStringField(TEXT("ObjectName")).Split("'", &ObjectType, &ObjectName);

	ObjectPath = PackageIndex->Get()->GetStringField(TEXT("ObjectPath"));
	ObjectPath.Split(".", &ObjectPath, nullptr);

	const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();

	/* Rare case of needing a GameName */
	if (!Settings->AssetSettings.GameName.IsEmpty()) {
		ObjectPath = ObjectPath.Replace(*(Settings->AssetSettings.GameName + "/Content"), TEXT("/Game"));
	}

	ObjectPath = ObjectPath.Replace(TEXT("Engine/Content"), TEXT("/Engine"));
	ObjectName = ObjectName.Replace(TEXT("'"), TEXT(""));

	if (ObjectName.Contains(".")) {
		ObjectName.Split(".", nullptr, &ObjectName);
	}

	if (ObjectName.Contains(".")) {
		ObjectName.Split(".", &Outer, &ObjectName);
	}

	return ObjectPath + "." + ObjectName;
}

/* Creates a plugin in the name (may result in bugs if inputted wrong) */
static void CreatePlugin(FString PluginName) {
	/* Plugin creation is different between UE5 and UE4 */
#if ENGINE_MAJOR_VERSION >= 5
	FPluginUtils::FNewPluginParamsWithDescriptor CreationParams;
	CreationParams.Descriptor.bCanContainContent = true;

	CreationParams.Descriptor.FriendlyName = PluginName;
	CreationParams.Descriptor.Version = 1;
	CreationParams.Descriptor.VersionName = TEXT("1.0");
	CreationParams.Descriptor.Category = TEXT("Other");

	FText FailReason;
	FPluginUtils::FLoadPluginParams LoadParams;
	LoadParams.bEnablePluginInProject = true;
	LoadParams.bUpdateProjectPluginSearchPath = true;
	LoadParams.bSelectInContentBrowser = false;

	FPluginUtils::CreateAndLoadNewPlugin(PluginName, FPaths::ProjectPluginsDir(), CreationParams, LoadParams);
#else
	FPluginUtils::FNewPluginParams CreationParams;
	CreationParams.bCanContainContent = true;

	FText FailReason;
	FPluginUtils::FMountPluginParams LoadParams;
	LoadParams.bEnablePluginInProject = true;
	LoadParams.bUpdateProjectPluginSearchPath = true;
	LoadParams.bSelectInContentBrowser = false;

	FPluginUtils::CreateAndMountNewPlugin(PluginName, FPaths::ProjectPluginsDir(), CreationParams, LoadParams, FailReason);
#endif

#define LOCTEXT_NAMESPACE "UMG"
#if WITH_EDITOR
	/* Setup notification's arguments */
	FFormatNamedArguments Args;
	Args.Add(TEXT("PluginName"), FText::FromString(PluginName));

	/* Create notification */
	FNotificationInfo Info(FText::Format(LOCTEXT("PluginCreated", "Plugin Created: {PluginName}"), Args));
	Info.ExpireDuration = 10.0f;
	Info.bUseLargeFont = true;
	Info.bUseSuccessFailIcons = false;
	Info.WidthOverride = FOptionalSize(350);
	SetNotificationSubText(Info, FText::FromString(FString("Created successfully")));
	
	TSharedPtr<SNotificationItem> NotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationPtr->SetCompletionState(SNotificationItem::CS_Success);
#endif
#undef LOCTEXT_NAMESPACE
}

inline void SendHttpRequest(const FString& URL, TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> OnComplete, const FString& Verb = "GET", const FString& ContentType = "", const FString& Content = "") {
	FHttpModule* Http = &FHttpModule::Get();
	if (!Http) {
		UE_LOG(LogTemp, Error, TEXT("HTTP module not available"));
		return;
	}

#if ENGINE_MAJOR_VERSION >= 5
	const TSharedRef<IHttpRequest> Request = Http->CreateRequest();
#else
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
#endif
	
	Request->SetURL(URL);
	Request->SetVerb(Verb);

	if (!ContentType.IsEmpty()) {
		Request->SetHeader(TEXT("Content-Type"), ContentType);
	}
    
	if (!Content.IsEmpty()) {
		Request->SetContentAsString(Content);
	}

	Request->OnProcessRequestComplete().BindLambda([OnComplete](const FHttpRequestPtr& RequestPtr, const FHttpResponsePtr& Response, const bool bWasSuccessful) {
		OnComplete(RequestPtr, Response, bWasSuccessful);
	});

	Request->ProcessRequest();
}