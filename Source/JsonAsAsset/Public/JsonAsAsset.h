/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Utilities/Compatibility.h"

#if ENGINE_UE4
#include "Modules/ModuleInterface.h"
#endif

struct GitHub {
    static inline FString URL = "https://github.com/JsonAsAsset/JsonAsAsset";
    
    struct README {
        static inline FString Link = URL + "?tab=readme-ov-file#asset-types";
        static inline FString AssetTypes = Link + "#asset-types";
        static inline FString Cloud = Link + "#cloud";
    };
};

struct Donation {
    static inline FString KO_FI = "https://ko-fi.com/t4ctor";
};

class FJsonAsAssetModule : public IModuleInterface {
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};