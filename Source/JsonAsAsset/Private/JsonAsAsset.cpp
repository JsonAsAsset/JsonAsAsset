/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "JsonAsAsset.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#if ENGINE_UE4
#include "ToolMenus.h"
#include "LevelEditor.h"
#endif

#include "Settings/JsonAsAssetSettings.h"
#include "MessageLogModule.h"

#include "Modules/UI/CommandsModule.h"
#include "Modules/UI/StyleModule.h"
#include "Toolbar/Toolbar.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifdef _MSC_VER
#undef GetObject
#endif

void FJsonAsAssetModule::StartupModule() {
    /* Initialize plugin style, reload textures, and register commands */
    FJsonAsAssetStyle::Initialize();
    FJsonAsAssetStyle::ReloadTextures();
    FJsonAsAssetCommands::Register();
	
    /* Register toolbar on startup */
	FJsonAsAssetToolbar Toolbar;

#if ENGINE_UE5
    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(&Toolbar, &FJsonAsAssetToolbar::Register));
#else
	{
    	const TSharedPtr<FUICommandList> PluginCommands = MakeShareable(new FUICommandList);

    	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    	const TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
    	ToolbarExtender->AddToolBarExtension(
			"Settings",
			EExtensionHook::After,
			PluginCommands,
			FToolBarExtensionDelegate::CreateRaw(&Toolbar, &FJsonAsAssetToolbar::UE4Register)
		);

    	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
#endif

    const UJsonAsAssetSettings* Settings = GetMutableDefault<UJsonAsAssetSettings>();

    /* Set up message log for JsonAsAsset */
    {
        FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
        FMessageLogInitializationOptions InitOptions;
        InitOptions.bShowPages = true;
        InitOptions.bAllowClear = true;
        InitOptions.bShowFilters = true;
        MessageLogModule.RegisterLogListing("JsonAsAsset", FText::FromString("JsonAsAsset"), InitOptions);
    }
	
	if (!Settings->Versioning.bDisable) {
		GJsonAsAssetVersioning.Update();
	}

	/* Update ExportDirectory if empty */
	if (Settings->Runtime.ExportDirectory.Path.IsEmpty()) {
		Settings->ReadAppData();
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

IMPLEMENT_MODULE(FJsonAsAssetModule, JsonAsAsset)
