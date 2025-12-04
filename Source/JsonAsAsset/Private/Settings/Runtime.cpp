/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Settings/Runtime.h"

#include "Misc/FileHelper.h"
#include "Utilities/EngineUtilities.h"

/* Define Global Struct */
FJRuntime GJsonAsAssetRuntime;

bool FJRuntime::IsOlderUE4Target() const {
	return MajorVersion == 4 && MinorVersion != -1 && MinorVersion < 14;
}

void FJRuntime::Update() {
	FString AppDataPath = FPlatformMisc::GetEnvironmentVariable(TEXT("APPDATA"));
	AppDataPath = FPaths::Combine(AppDataPath, TEXT("FModel/AppSettings.json"));

	if (FString FileContent; FFileHelper::LoadFileToString(FileContent, *AppDataPath)) {
		if (TSharedPtr<FJsonObject> JsonObject; DeserializeJSONObject(FileContent, JsonObject)) {
			ExportDirectory.Path = JsonObject->GetStringField(TEXT("PropertiesDirectory")).Replace(TEXT("\\"), TEXT("/"));
		}
	}
}
