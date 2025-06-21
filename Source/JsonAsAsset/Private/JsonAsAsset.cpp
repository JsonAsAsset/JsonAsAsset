/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "JsonAsAsset.h"

#include "./Importers/Constructor/Importer.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#if !ENGINE_UE5
/* ReSharper disable once CppUnusedIncludeDirective */
#include "MainFrame/Public/Interfaces/IMainFrameModule.h"
#endif

#if ENGINE_UE4
#include "ToolMenus.h"
#include "LevelEditor.h"
#endif

#include "Settings/JsonAsAssetSettings.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "ISettingsModule.h"
#include "MessageLogModule.h"

/* Settings */
#include "./Settings/Details/JsonAsAssetSettingsDetails.h"

/* ReSharper disable once CppUnusedIncludeDirective */
#include "HttpModule.h"

/* ReSharper disable once CppUnusedIncludeDirective */
#include "Modules/Tools/SkeletalMeshData.h"

#include "Modules/UI/CommandsModule.h"
#include "Modules/UI/StyleModule.h"
#include "Toolbar/Toolbar.h"
#include "Utilities/Compatibility.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifdef _MSC_VER
#undef GetObject
#endif

#if PLATFORM_WINDOWS
    static TWeakPtr<SNotificationItem> ImportantNotificationPtr;
#endif

void FJsonAsAssetModule::StartupModule() {
    /* Initialize plugin style, reload textures, and register commands */
    FJsonAsAssetStyle::Initialize();
    FJsonAsAssetStyle::ReloadTextures();
    FJsonAsAssetCommands::Register();

    /* Set up plugin command list and map actions */
    PluginCommands = MakeShareable(new FUICommandList);

    /* Register menus on startup */
    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FJsonAsAssetModule::RegisterMenus));

    /* Check for export directory in settings */
    Settings = GetMutableDefault<UJsonAsAssetSettings>();
	
	if (!IsSetup(Settings)) {
	    const FText TitleText = FText::FromString("Setup JsonAsAsset Settings");
	    const FText MessageText = FText::FromString(
	        "JsonAsAsset requires a proper setup to run properly.\n\nOpen the documentation for JsonAsAsset for more information."
	    );

	    FNotificationInfo Info(TitleText);
		SetNotificationSubText(Info, MessageText);
		
	    /* Notification settings */
	    Info.bFireAndForget = false;
	    Info.FadeOutDuration = 3.0f;
	    Info.ExpireDuration = 0.0f;
	    Info.bUseLargeFont = false;
	    Info.bUseThrobber = false;

		Info.ButtonDetails.Add(
			FNotificationButtonInfo(
				FText::FromString("Open Documentation"),
				FText::GetEmpty(),
				FSimpleDelegate::CreateStatic([]() {
					const FString URL = "https://github.com/JsonAsAsset/JsonAsAsset";
					FPlatformProcess::LaunchURL(*URL, nullptr, nullptr); 
				})
			)
		);

		/* Add button to open plugin settings */
	    Info.ButtonDetails.Add(
	        FNotificationButtonInfo(
	            FText::FromString("Open Settings"),
	            FText::GetEmpty(),
	            FSimpleDelegate::CreateStatic([]() {
	                const TSharedPtr<SNotificationItem> NotificationItem = ImportantNotificationPtr.Pin();
	                if (NotificationItem.IsValid()) {
	                    NotificationItem->Fadeout();
	                    ImportantNotificationPtr.Reset();
	                }

	                /* Navigate to plugin settings */
	                FModuleManager::LoadModuleChecked<ISettingsModule>("Settings")
	                    .ShowViewer("Editor", "Plugins", "JsonAsAsset");
	            })
	        )
	    );

	    ImportantNotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);
	    ImportantNotificationPtr.Pin()->SetCompletionState(SNotificationItem::CS_Pending);
	}

    /* Set up message log for JsonAsAsset */
    {
        FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
        FMessageLogInitializationOptions InitOptions;
        InitOptions.bShowPages = true;
        InitOptions.bAllowClear = true;
        InitOptions.bShowFilters = true;
        MessageLogModule.RegisterLogListing("JsonAsAsset", NSLOCTEXT("JsonAsAsset", "JsonAsAssetLogLabel", "JsonAsAsset"), InitOptions);
    }

#if ENGINE_UE4
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor"); {
    	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
    	ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FJsonAsAssetModule::AddToolbarExtension));

    	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
#endif

    /* Register custom class layout for settings */
    FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.RegisterCustomClassLayout(UJsonAsAssetSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FJsonAsAssetSettingsDetails::MakeInstance));

	Plugin = IPluginManager::Get().FindPlugin("JsonAsAsset");

	GJsonAsAssetVersioning.Update();
}

void FJsonAsAssetModule::ShutdownModule() {
	/* Unregister startup callback and tool menus */
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	/* Shutdown the plugin style and unregister commands */
	FJsonAsAssetStyle::Shutdown();
	FJsonAsAssetCommands::Unregister();

	/* Unregister message log listing if the module is loaded */
	if (FModuleManager::Get().IsModuleLoaded("MessageLog")) {
		FMessageLogModule& MessageLogModule = FModuleManager::GetModuleChecked<FMessageLogModule>("MessageLog");
		MessageLogModule.UnregisterLogListing("JsonAsAsset");
	}
}

void FJsonAsAssetModule::RegisterMenus() {
	Toolbar.Register();
}

#if ENGINE_UE4
void FJsonAsAssetModule::AddToolbarExtension(FToolBarBuilder& Builder) {
	Toolbar.UE4Register(Builder);
}
#endif

bool FJsonAsAssetModule::IsSetup(const UJsonAsAssetSettings* SettingsReference, TArray<FString>& Params) {
	if (SettingsReference->ExportDirectory.Path.IsEmpty()) {
		Params.Add("Export Directory is missing");
	}

	return !SettingsReference->ExportDirectory.Path.IsEmpty();
}

bool FJsonAsAssetModule::IsSetup(const UJsonAsAssetSettings* SettingsReference) {
	TArray<FString> Params;
	return IsSetup(SettingsReference, Params);
}

IMPLEMENT_MODULE(FJsonAsAssetModule, JsonAsAsset)
