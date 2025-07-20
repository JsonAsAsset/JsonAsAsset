/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class UJsonAsAssetSettings;

/* Details Widget For JsonAsAssetSettings */
class FJsonAsAssetSettingsDetails final : public IDetailCustomization {
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	void EditConfiguration(IDetailLayoutBuilder& DetailBuilder);
};