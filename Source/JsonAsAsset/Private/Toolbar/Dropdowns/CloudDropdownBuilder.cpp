/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Toolbar/Dropdowns/CloudDropdownBuilder.h"
#include "Utilities/EngineUtilities.h"

void ICloudDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	UJsonAsAssetSettings* Settings = GetSettings();
	
	/* Cloud must be enabled, and if there is an action required, don't create Cloud's dropdown */
	if (!Settings->bEnableCloudServer || !UJsonAsAssetSettings::IsSetup(Settings)) {
		return;
	}
	
	MenuBuilder.BeginSection("JsonAsAssetSection", FText::FromString("Cloud"));

	MenuBuilder.AddMenuEntry(
		FText::FromString("Learn how to setup"),
		FText::FromString(""),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]() {
				const FString URL = "https://github.com/JsonAsAsset/JsonAsAsset?tab=readme-ov-file#cloud";
				FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
			})
		)
	);
	
	MenuBuilder.EndSection();
}
