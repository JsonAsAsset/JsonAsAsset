/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Settings/JsonAsAssetSettings.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"

FName GJsonAsAssetName = FName("JsonAsAsset");
FName GJsonAsAssetSettingsCategoryName = FName("General");

UJsonAsAssetSettings::UJsonAsAssetSettings() {
	CategoryName = GJsonAsAssetSettingsCategoryName;
	SectionName = GJsonAsAssetName;
}

FText UJsonAsAssetSettings::GetSectionText() const {
	return FText::FromString(GJsonAsAssetName.ToString());
}