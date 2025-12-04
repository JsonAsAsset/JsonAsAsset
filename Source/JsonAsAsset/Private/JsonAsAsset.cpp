/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "JsonAsAsset.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#if ENGINE_UE4
#include "ToolMenus.h"
#include "LevelEditor.h"
#endif

#include "Modules/Versioning.h"

#include "Modules/UI/StyleModule.h"
#include "Toolbar/Toolbar.h"
#include "Utilities/EngineUtilities.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifdef _MSC_VER
#undef GetObject
#endif

void FJsonAsAssetModule::StartupModule() {
    /* Initialize plugin style, reload textures, and register commands */
    FJsonAsAssetStyle::Initialize();
    FJsonAsAssetStyle::ReloadTextures();
	
    /* Register Toolbar on startup */
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

    /* Set up message log for JsonAsAsset */
    {
        FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
        FMessageLogInitializationOptions InitOptions;
        InitOptions.bShowPages = true;
        InitOptions.bAllowClear = true;
        InitOptions.bShowFilters = true;
        MessageLogModule.RegisterLogListing(GJsonAsAssetName, FText::FromString(GJsonAsAssetName.ToString()), InitOptions);
    }

    const UJsonAsAssetSettings* Settings = GetSettings();
	
	if (!Settings->Versioning.bDisable) {
		GJsonAsAssetVersioning.Update();
	}
}

void FJsonAsAssetModule::ShutdownModule() {
	/* Unregister startup callback and tool menus */
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	/* Shutdown the plugin style */
	FJsonAsAssetStyle::Shutdown();

	/* Unregister message log listing if the module is loaded */
	if (FModuleManager::Get().IsModuleLoaded("MessageLog")) {
		FMessageLogModule& MessageLogModule = FModuleManager::GetModuleChecked<FMessageLogModule>("MessageLog");
		MessageLogModule.UnregisterLogListing(GJsonAsAssetName);
	}
}

IMPLEMENT_MODULE(FJsonAsAssetModule, JsonAsAsset)
