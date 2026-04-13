/* Copyright JsonAsAsset Contributors 2024-2026 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/Toolbar/Tools/SelectedAssetsBase.h"

class JSONASASSET_API TCurveLinearColorData : public TSelectedAssetsBase {
public:
	virtual void Process(UObject* Object) override;
};