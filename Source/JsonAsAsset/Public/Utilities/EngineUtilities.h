/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Utilities/Serializers/PropertyUtilities.h"
#include "Windows/WindowsPlatformApplicationMisc.h"
#include "Utilities/Serializers/ObjectUtilities.h"
#include "Settings/JsonAsAssetSettings.h"
#include "Interfaces/IMainFrameModule.h"
#include "IContentBrowserSingleton.h"
#include "Windows/WindowsHWrapper.h"
#include "Interfaces/IHttpRequest.h"
#include "DesktopPlatformModule.h"
#include "ContentBrowserModule.h"
#include "IDesktopPlatform.h"
#include "RemoteUtilities.h"
#include "AssetUtilities.h"
#include "PluginUtils.h"
#include "HttpModule.h"
#include "IMessageLogListing.h"
#include "TlHelp32.h"
#include "Json.h"
#include "MessageLogModule.h"

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
T* GetSelectedAsset(const bool SuppressErrors = false, FString OptionalAssetNameCheck = "") {
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

	if (SelectedAssets.Num() == 0) {
		if (SuppressErrors == true) {
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
		if (SuppressErrors == true) {
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

	if (CastedAsset && OptionalAssetNameCheck != "" && !CastedAsset->GetName().Equals(OptionalAssetNameCheck)) {
		CastedAsset = nullptr;
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
	const FText DialogMessage = FText::FromString(Text);

	FMessageDialog::Open(EAppMsgType::Ok, DialogMessage);
}

inline auto SpawnYesNoPrompt = [](const FString& Title, const FString& Text, const TFunction<void(bool)>& OnResponse) {
	const FText DialogTitle = FText::FromString(Title);
	const FText DialogMessage = FText::FromString(Text);

	const EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgType::YesNo, DialogMessage, &DialogTitle);

	OnResponse(Response == EAppReturnType::Yes);
};

/* Gets all assets in selected folder */
inline TArray<FAssetData> GetAssetsInSelectedFolder() {
	TArray<FAssetData> AssetDataList;

	/* Get the Content Browser Module */
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	TArray<FString> SelectedFolders;
	ContentBrowserModule.Get().GetSelectedPathViewFolders(SelectedFolders);

	if (SelectedFolders.Num() == 0) {
		UE_LOG(LogTemp, Warning, TEXT("No folder selected in the Content Browser."));
		return AssetDataList; 
	}

	const FString CurrentFolder = SelectedFolders[0];

	/* Check if the folder is the root folder, and show a prompt if */
	if (CurrentFolder == "/Game") {
		bool bContinue = false;
		
		SpawnYesNoPrompt(
			TEXT("Large Operation"),
			TEXT("This will stall the editor for a long time. Continue anyway?"),
			[&](const bool bConfirmed) {
				bContinue = bConfirmed;
			}
		);

		if (!bContinue) {
			UE_LOG(LogTemp, Warning, TEXT("Action cancelled by user."));
			return AssetDataList;
		}
	}

	/* Get the Asset Registry Module */
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
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

	const HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Snapshot != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 ProcessEntry;
		ProcessEntry.dwSize = sizeof(ProcessEntry);

		if (Process32First(Snapshot, &ProcessEntry)) {
			do {
				if (_wcsicmp(ProcessEntry.szExeFile, ProcessNameChar) == 0) {
					bIsRunning = true;
					break;
				}
			} while (Process32Next(Snapshot, &ProcessEntry));
		}

		CloseHandle(Snapshot);
	}

	return bIsRunning;
}

inline TSharedPtr<FJsonObject> GetExport(const FString& Type, TArray<TSharedPtr<FJsonValue>> AllJsonObjects, const bool bGetProperties = false) {
	for (const TSharedPtr<FJsonValue> Value : AllJsonObjects) {
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

inline TSharedPtr<FJsonObject> GetExportByName(const FString& Name, TArray<TSharedPtr<FJsonValue>> AllJsonObjects, const bool bGetProperties = false) {
	for (const TSharedPtr<FJsonValue> Value : AllJsonObjects) {
		const TSharedPtr<FJsonObject> ValueObject = Value->AsObject();

		if (ValueObject->GetStringField(TEXT("Name")) == Name) {
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

inline bool DeserializeArrayJSON(const FString& String, TArray<TSharedPtr<FJsonValue>>& JsonParsed) {
	FString Content = FString(TEXT("{\"data\": "));
	Content.Append(String);
	Content.Append(FString("}"));

	const TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Content);

	TSharedPtr<FJsonObject> JsonObject;
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject)) {
		JsonParsed = JsonObject->GetArrayField(TEXT("data"));
	
		return true;
	}

	return false;
}

inline bool DeserializeJSONObject(const FString& String, TSharedPtr<FJsonObject>& JsonParsed) {
	FString Content = FString(TEXT("{\"data\": "));
	Content.Append(String);
	Content.Append(FString("}"));

	const TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Content);

	TSharedPtr<FJsonObject> JsonObject;
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject)) {
		JsonParsed = JsonObject->GetObjectField(TEXT("data"));
	
		return true;
	}

	return false;
}

inline TArray<FString> OpenFileDialog(const FString& Title, const FString& Type) {
	TArray<FString> ReturnValue;

	/* Window Handler for Windows */
	const void* ParentWindowHandle = nullptr;
	const IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
	const TSharedPtr<SWindow> MainWindow = MainFrameModule.GetParentWindow();

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
		constexpr uint32 SelectionFlag = 1;
		
		DesktopPlatform->OpenFileDialog(ParentWindowHandle, Title, DefaultPath, FString(""), Type, SelectionFlag, ReturnValue);
	}

	return ReturnValue;
}

inline FString OpenFolderDialog(const FString& Title, const FString& DefaultPath) {
	FString OutFolder;
	const void* ParentWindowHandle = nullptr;

	const IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
	const TSharedPtr<SWindow> MainWindow = MainFrameModule.GetParentWindow();
	if (MainWindow.IsValid() && MainWindow->GetNativeWindow().IsValid()) {
		ParentWindowHandle = MainWindow->GetNativeWindow()->GetOSWindowHandle();
	}

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform) {
		DesktopPlatform->OpenDirectoryDialog(ParentWindowHandle, Title, DefaultPath, OutFolder);
	}

	return OutFolder;
}

inline TArray<FString> OpenFolderDialog(const FString& Title) {
	TArray<FString> ReturnValue;

	/* Window Handler for Windows */
	const void* ParentWindowHandle = nullptr;

	const IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
	const TSharedPtr<SWindow> MainWindow = MainFrameModule.GetParentWindow();

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
	
#if ENGINE_UE5
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
#if ENGINE_UE5
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
#if ENGINE_UE5
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

	const TSharedPtr<SNotificationItem> NotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);
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

#if ENGINE_UE5
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

inline UObjectSerializer* CreateObjectSerializer() {
	UPropertySerializer* PropertySerializer = NewObject<UPropertySerializer>();
	UObjectSerializer* ObjectSerializer = NewObject<UObjectSerializer>();
	ObjectSerializer->SetPropertySerializer(PropertySerializer);

	return ObjectSerializer;
}

inline TArray<TSharedPtr<FJsonValue>> GetExportsStartingWith(const FString& Start, const FString& Property, TArray<TSharedPtr<FJsonValue>> AllJsonObjects) {
	TArray<TSharedPtr<FJsonValue>> FilteredObjects;

	for (const TSharedPtr<FJsonValue>& JsonObjectValue : AllJsonObjects) {
		if (JsonObjectValue->Type == EJson::Object) {
			TSharedPtr<FJsonObject> JsonObject = JsonObjectValue->AsObject();

			if (JsonObject.IsValid() && JsonObject->HasField(Property)) {
				const FString StringValue = JsonObject->GetStringField(Property);

				/* Check if the "Type" field starts with the specified string */
				if (StringValue.StartsWith(Start)) {
					FilteredObjects.Add(JsonObjectValue);
				}
			}
		}
	}

	return FilteredObjects;
}

inline TSharedPtr<FJsonObject> GetExportStartingWith(const FString& Start, const FString& Property, TArray<TSharedPtr<FJsonValue>> AllJsonObjects, const bool bExportProperties = false) {
	for (const TSharedPtr<FJsonValue>& JsonObjectValue : AllJsonObjects) {
		if (JsonObjectValue->Type == EJson::Object) {
			TSharedPtr<FJsonObject> JsonObject = JsonObjectValue->AsObject();

			if (JsonObject.IsValid() && JsonObject->HasField(Property)) {
				const FString StringValue = JsonObject->GetStringField(Property);

				/* Check if the "Type" field starts with the specified string */
				if (StringValue.StartsWith(Start)) {
					if (bExportProperties) {
						if (JsonObject->HasField(TEXT("Properties"))) {
							return JsonObject->GetObjectField(TEXT("Properties"));
						}
					}
					return JsonObject;
				}
			}
		}
	}

	return TSharedPtr<FJsonObject>();
}

inline TSharedPtr<FJsonObject> GetExportMatchingWith(const FString& Match, const FString& Property, TArray<TSharedPtr<FJsonValue>> AllJsonObjects, const bool bExportProperties = false) {
	for (const TSharedPtr<FJsonValue>& JsonObjectValue : AllJsonObjects) {
		if (JsonObjectValue->Type == EJson::Object) {
			TSharedPtr<FJsonObject> JsonObject = JsonObjectValue->AsObject();

			if (JsonObject.IsValid() && JsonObject->HasField(Property)) {
				const FString StringValue = JsonObject->GetStringField(Property);

				/* Check if the "Name" field starts with the specified string */
				if (StringValue.Equals(Match)) {
					if (bExportProperties) {
						if (JsonObject->HasField(TEXT("Properties"))) {
							return JsonObject->GetObjectField(TEXT("Properties"));
						}
					}
					return JsonObject;
				}
			}
		}
	}

	return TSharedPtr<FJsonObject>();
}

/* Helper Math functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
inline TSharedPtr<FJsonObject> GetVectorJson(const FVector& Vec) {
	TSharedPtr<FJsonObject> OutVec = MakeShareable(new FJsonObject);
	
	OutVec->SetNumberField(TEXT("X"), Vec.X);
	OutVec->SetNumberField(TEXT("Y"), Vec.Y);
	OutVec->SetNumberField(TEXT("Z"), Vec.Z);
	
	return OutVec;
}

inline TSharedPtr<FJsonObject> GetRotationJson(const FQuat& Quat) {
	TSharedPtr<FJsonObject> OutRot = MakeShareable(new FJsonObject);
	
	OutRot->SetNumberField(TEXT("X"), Quat.X);
	OutRot->SetNumberField(TEXT("Y"), Quat.Y);
	OutRot->SetNumberField(TEXT("Z"), Quat.Z);
	OutRot->SetNumberField(TEXT("W"), Quat.W);
	OutRot->SetBoolField(TEXT("IsNormalized"), true);
	OutRot->SetNumberField(TEXT("Size"), Quat.Size());
	OutRot->SetNumberField(TEXT("SizeSquared"), Quat.SizeSquared());
	
	return OutRot;
}

inline TSharedPtr<FJsonObject> GetTransformJson(const FTransform& Transform) {
	TSharedPtr<FJsonObject> OutTransform = MakeShareable(new FJsonObject);
	
	OutTransform->SetObjectField(TEXT("Rotation"), GetRotationJson(Transform.GetRotation()));
	OutTransform->SetObjectField(TEXT("Translation"), GetVectorJson(Transform.GetTranslation()));
	OutTransform->SetObjectField(TEXT("Scale3D"), GetVectorJson(Transform.GetScale3D()));
	
	return OutTransform;
}

inline FTransform GetTransformFromJson(const TSharedPtr<FJsonObject>& JsonObject) {
	FTransform OutTransform = FTransform::Identity;
	
	if (!JsonObject.IsValid()) {
		return OutTransform;
	}

	if (JsonObject->HasField(TEXT("Rotation"))) {
		const TSharedPtr<FJsonObject> QuatObject = JsonObject->GetObjectField(TEXT("Rotation"));
		
		FQuat Quat;
		Quat.X = QuatObject->GetNumberField(TEXT("X"));
		Quat.Y = QuatObject->GetNumberField(TEXT("Y"));
		Quat.Z = QuatObject->GetNumberField(TEXT("Z"));
		Quat.W = QuatObject->GetNumberField(TEXT("W"));
		
		OutTransform.SetRotation(Quat);
	}

	if (JsonObject->HasField(TEXT("Translation"))) {
		const TSharedPtr<FJsonObject> TranslationObject = JsonObject->GetObjectField(TEXT("Translation"));
		FVector Translation;
		
		Translation.X = TranslationObject->GetNumberField(TEXT("X"));
		Translation.Y = TranslationObject->GetNumberField(TEXT("Y"));
		Translation.Z = TranslationObject->GetNumberField(TEXT("Z"));
		
		OutTransform.SetTranslation(Translation);
	}

	if (JsonObject->HasField(TEXT("Scale3D"))) {
		const TSharedPtr<FJsonObject> ScaleObj = JsonObject->GetObjectField(TEXT("Scale3D"));
		FVector Scale;
		
		Scale.X = ScaleObj->GetNumberField(TEXT("X"));
		Scale.Y = ScaleObj->GetNumberField(TEXT("Y"));
		Scale.Z = ScaleObj->GetNumberField(TEXT("Z"));
		
		OutTransform.SetScale3D(Scale);
	}

	return OutTransform;
}

#if ENGINE_UE4 && (!UE4_27_BELOW)
inline UStructProperty* LoadStructProperty(const TSharedPtr<FJsonObject>& JsonObject) {
#else
inline FStructProperty* LoadStructProperty(const TSharedPtr<FJsonObject>& JsonObject) {
#endif
    if (!JsonObject.IsValid()) {
        return nullptr;
    }

    FString ObjectName;
    if (!JsonObject->TryGetStringField(TEXT("ObjectName"), ObjectName)) {
        return nullptr;
    }

    const int32 FirstQuoteIndex = ObjectName.Find(TEXT("'"));
    const int32 LastQuoteIndex = ObjectName.Find(TEXT("'"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	
    if (FirstQuoteIndex == INDEX_NONE || LastQuoteIndex == INDEX_NONE || LastQuoteIndex <= FirstQuoteIndex) {
        return nullptr;
    }

    const FString InnerString = ObjectName.Mid(FirstQuoteIndex + 1, LastQuoteIndex - FirstQuoteIndex - 1);

    FString StructName, PropertyName;
    if (!InnerString.Split(TEXT(":"), &StructName, &PropertyName)) {
        return nullptr;
    }

    FString ObjectPath = JsonObject->GetStringField(TEXT("ObjectPath"));

    const UStruct* StructDef = FindObject<UStruct>(ANY_PACKAGE, *StructName);
    if (!StructDef) {
        return nullptr;
    }

#if ENGINE_UE4 && (!UE4_27_BELOW)
    UStructProperty* StructProp = FindFProperty<UStructProperty>(StructDef, *PropertyName);
#else
    FStructProperty* StructProp = FindFProperty<FStructProperty>(StructDef, *PropertyName);
#endif
    if (!StructProp) {
        return nullptr;
    }

	return StructProp;
}

inline void CloseApplicationByProcessName(const FString& ProcessName) {
	DWORD ProcessID = 0;

	const HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (Snapshot != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 ProcessEntry;
		ProcessEntry.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(Snapshot, &ProcessEntry)) {
			do {
				if (FCString::Stricmp(ProcessEntry.szExeFile, ProcessName.GetCharArray().GetData()) == 0) {
					ProcessID = ProcessEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(Snapshot, &ProcessEntry));
		}
		
		CloseHandle(Snapshot);
	}

	if (ProcessID != 0) {
		const HANDLE Process = OpenProcess(PROCESS_TERMINATE, false, ProcessID);
		
		if (Process != nullptr) {
			TerminateProcess(Process, 0);
			CloseHandle(Process);
		}
	}
}

inline TSharedPtr<FJsonObject> RequestObjectURL(const FString& URL) {
	FHttpModule* HttpModule = &FHttpModule::Get();

	const auto Request = HttpModule->CreateRequest();
			
	Request->SetURL(URL);
	Request->SetVerb(TEXT("GET"));

	const auto Response = FRemoteUtilities::ExecuteRequestSync(Request);
	if (!Response.IsValid()) return TSharedPtr<FJsonObject>();

	TSharedPtr<FJsonObject> DeserializedJSON;

	if (!DeserializeJSONObject(Response->GetContentAsString(), DeserializedJSON)) return TSharedPtr<FJsonObject>();
	return DeserializedJSON;
};

inline TArray<TSharedPtr<FJsonValue>> RequestArrayURL(const FString& URL) {
	FHttpModule* HttpModule = &FHttpModule::Get();

	const auto Request = HttpModule->CreateRequest();
			
	Request->SetURL(URL);
	Request->SetVerb(TEXT("GET"));

	const auto Response = FRemoteUtilities::ExecuteRequestSync(Request);
	if (!Response.IsValid()) return TArray<TSharedPtr<FJsonValue>>();

	TArray<TSharedPtr<FJsonValue>> DeserializedJSON;

	if (!DeserializeArrayJSON(Response->GetContentAsString(), DeserializedJSON)) return TArray<TSharedPtr<FJsonValue>>();
	return DeserializedJSON;
};

inline TSubclassOf<UObject> LoadClassFromPath(const FString& ObjectName, const FString& ObjectPath) {
	const FString FullPath = ObjectPath + TEXT(".") + ObjectName;
	UObject* LoadedObject = StaticLoadObject(UObject::StaticClass(), nullptr, *FullPath);

	if (LoadedObject) {
		UClass* LoadedClass = Cast<UClass>(LoadedObject);
		if (LoadedClass) {
			return LoadedClass;
		}
	}

	return nullptr;
}

inline TSubclassOf<UObject> LoadBlueprintClass(FString& ObjectPath) {
	const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();
	
	if (!Settings->AssetSettings.GameName.IsEmpty()) {
		ObjectPath = ObjectPath.Replace(*(Settings->AssetSettings.GameName + "/Content"), TEXT("/Game"));
	}
	
	FString FullPath = ObjectPath; 
	if (FullPath.EndsWith(TEXT(".1"))) {
		FullPath = FullPath.LeftChop(2);
	}

	UObject* LoadedObject = StaticLoadObject(UObject::StaticClass(), nullptr, *FullPath);

	if (LoadedObject) {
		const UBlueprint* LoadedBlueprint = Cast<UBlueprint>(LoadedObject);
		
		if (LoadedBlueprint && LoadedBlueprint->GeneratedClass) {
			return LoadedBlueprint->GeneratedClass;
		}
	}

	return nullptr;
}

inline UClass* LoadClass(const TSharedPtr<FJsonObject>& SuperStruct) {
	const FString ObjectName = SuperStruct->GetStringField("ObjectName").Replace(TEXT("Class'"), TEXT("")).Replace(TEXT("'"), TEXT(""));
	FString ObjectPath = SuperStruct->GetStringField("ObjectPath");

	/* It's a C++ class if it has Script in it */
	if (ObjectPath.Contains("/Script/")) {
		return LoadClassFromPath(ObjectName, ObjectPath);
	}
	
	ObjectPath.Split(".", &ObjectPath, nullptr);

	return LoadBlueprintClass(ObjectPath);
}

inline void RedirectPath(FString& OutPath) {
	const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();

	for (FJPathRedirector Redirector : Settings->AssetSettings.PathRedirectors) {
		OutPath = OutPath.Replace(*Redirector.Source, *Redirector.Target);
	}
}

inline TSharedRef<IMessageLogListing> GetMessageLog() {
	FMessageLogModule& MessageLogModule = FModuleManager::GetModuleChecked<FMessageLogModule>("MessageLog");
	const TSharedRef<IMessageLogListing> LogListing = MessageLogModule.GetLogListing("JsonAsAsset");

	return LogListing;
}

inline void EmptyMessageLog() {
	GetMessageLog()->ClearMessages();
}

inline void RemoveNotification(TWeakPtr<SNotificationItem> Notification) {
	const TSharedPtr<SNotificationItem> Item = Notification.Pin();

	if (Item.IsValid()) {
		Item->SetFadeOutDuration(0.001);
		Item->Fadeout();
		Notification.Reset();
	}
}

inline UJsonAsAssetSettings* GetSettings() {
	return GetMutableDefault<UJsonAsAssetSettings>();
}