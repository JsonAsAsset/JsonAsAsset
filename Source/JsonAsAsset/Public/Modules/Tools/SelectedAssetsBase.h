/* Copyright JsonAsAsset Contributors 2024-2026 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/Tools/ToolBase.h"

class JSONASASSET_API TSelectedAssetsBase : public TToolBase {
public:
	virtual void Execute();
	virtual void Process(UObject* Object) { }

	static TArray<TSharedPtr<FJsonValue>> SendToCloudForExports(const FString& ObjectPath);
};