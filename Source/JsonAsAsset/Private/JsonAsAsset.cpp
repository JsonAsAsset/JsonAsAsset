/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "JsonAsAsset.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#if ENGINE_UE4
#include "ToolMenus.h"
#include "LevelEditor.h"
#endif

#include "Http.h"
#include "Modules/Versioning.h"

#include "Modules/UI/StyleModule.h"
#include "Modules/Toolbar/Toolbar.h"
#include "Utilities/EngineUtilities.h"

#include "Logging/LogVerbosity.h"
#include "Logging/LogMacros.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifdef _MSC_VER
#undef GetObject
#endif

void FJsonAsAssetModule::StartupModule() {
	LogHttp.SetVerbosity(ELogVerbosity::Error);

	FJMetadata::Initialize();
	
    /* Initialize plugin style, reload textures */
    FJsonAsAssetStyle::Initialize();
    FJsonAsAssetStyle::ReloadTextures();

    /* Register Toolbar */
#if ENGINE_UE5
	UJsonAsAssetToolbar* Toolbar = NewObject<UJsonAsAssetToolbar>();
	
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateUObject(Toolbar, &UJsonAsAssetToolbar::Register));
#else
	{
    	const TSharedPtr<FUICommandList> PluginCommands = MakeShareable(new FUICommandList);

    	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    	const TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
    	ToolbarExtender->AddToolBarExtension(
			"Settings",
			EExtensionHook::After,
			PluginCommands,
			FToolBarExtensionDelegate::CreateStatic(&UJsonAsAssetToolbar::UE4Register)
		);

    	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
#endif

    const UJsonAsAssetSettings* Settings = GetSettings();
	
	if (!Settings->Versioning.Disable) {
		GJsonAsAssetVersioning.Update();
	}
}

void FJsonAsAssetModule::ShutdownModule() {
	/* Unregister startup callback and tool menus */
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	/* Shutdown the plugin style */
	FJsonAsAssetStyle::Shutdown();
}

IMPLEMENT_MODULE(FJsonAsAssetModule, JsonAsAsset)
