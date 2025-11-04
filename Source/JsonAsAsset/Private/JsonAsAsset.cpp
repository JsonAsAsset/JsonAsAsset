/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "JsonAsAsset.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#if ENGINE_UE4
/* ReSharper disable once CppUnusedIncludeDirective */
#include "MainFrame/Public/Interfaces/IMainFrameModule.h"

#include "ToolMenus.h"
#include "LevelEditor.h"
#endif

#include "Settings/JsonAsAssetSettings.h"
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

void FJsonAsAssetModule::StartupModule() {
    /* Initialize plugin style, reload textures, and register commands */
    FJsonAsAssetStyle::Initialize();
    FJsonAsAssetStyle::ReloadTextures();
    FJsonAsAssetCommands::Register();

    /* Set up plugin command list and map actions */
    PluginCommands = MakeShareable(new FUICommandList);

    /* Register menus on startup */
    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FJsonAsAssetModule::RegisterMenus));

    Settings = GetMutableDefault<UJsonAsAssetSettings>();

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
    {
	    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	    const TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
    	ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FJsonAsAssetModule::AddToolbarExtension));

    	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
#endif

	Plugin = IPluginManager::Get().FindPlugin("JsonAsAsset");

	if (!Settings->Versioning.bDisable) {
		GJsonAsAssetVersioning.Update();
	}

	/* Update ExportDirectory if empty */
	if (Settings->ExportDirectory.Path.IsEmpty()) {
		FJsonAsAssetSettingsDetails::ReadConfiguration();
	}
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

IMPLEMENT_MODULE(FJsonAsAssetModule, JsonAsAsset)
