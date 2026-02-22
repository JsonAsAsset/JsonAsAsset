/* Copyright JsonAsAsset Contributors 2024-2026 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/Tools/SelectedAssetsBase.h"

class JSONASASSET_API TWidgetAnimations : public TSelectedAssetsBase {
public:
	virtual void Process(UObject* Object) override;
};