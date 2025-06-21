/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/Versioning.h"
#include "Toolbar/Toolbar.h"
#include "Utilities/Compatibility.h"

#if ENGINE_UE4
#include "Modules/ModuleInterface.h"
#endif

class UJsonAsAssetSettings;

class FJsonAsAssetModule : public IModuleInterface {
public:
	FJsonAsAssetToolbar Toolbar;
    
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    void RegisterMenus();

    TSharedPtr<FUICommandList> PluginCommands;
    UJsonAsAssetSettings* Settings = nullptr;
    TSharedPtr<IPlugin> Plugin;

#if ENGINE_UE4
    void AddToolbarExtension(FToolBarBuilder& Builder);
#endif
};