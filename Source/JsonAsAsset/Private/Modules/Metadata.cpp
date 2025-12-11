/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Modules/Metadata.h"

#include "Interfaces/IPluginManager.h"
#include "Utilities/EngineUtilities.h"

FName GJsonAsAssetName = FName("JsonAsAsset");

TSharedPtr<IPlugin> FJMetadata::Plugin = nullptr;
FString FJMetadata::Version = "";

void FJMetadata::Initialize() {
    Plugin = GetPlugin(GJsonAsAssetName.ToString());
    Version = Plugin->GetDescriptor().VersionName;
}
