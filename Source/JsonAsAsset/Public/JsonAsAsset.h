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
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    TSharedPtr<FUICommandList> PluginCommands;
    UJsonAsAssetSettings* Settings = nullptr;
    TSharedPtr<IPlugin> Plugin;

	FJsonAsAssetToolbar Toolbar;
};