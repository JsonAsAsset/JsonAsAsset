/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/Versioning.h"
#include "Utilities/Compatibility.h"

#if ENGINE_UE4
#include "Modules/ModuleInterface.h"
#endif

class UJsonAsAssetSettings;

class FJsonAsAssetModule : public IModuleInterface {
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

};