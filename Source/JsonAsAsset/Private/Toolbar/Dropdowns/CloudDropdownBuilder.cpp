/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Toolbar/Dropdowns/CloudDropdownBuilder.h"

#include "JsonAsAsset.h"
#include "Utilities/EngineUtilities.h"

void ICloudDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	const UJsonAsAssetSettings* Settings = GetSettings();
	
	/* Cloud must be enabled, and if there is an action required, don't create Cloud's dropdown */
	if (!Settings->bEnableCloudServer || !Settings->IsValid()) {
		return;
	}
	
	MenuBuilder.BeginSection("JsonAsAssetCloudSection", FText::FromString("Cloud"));

	MenuBuilder.AddMenuEntry(
		FText::FromString("Learn how to setup"),
		FText::FromString(""),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "SessionFrontEnd.TabIcon"),
		FUIAction(
			FExecuteAction::CreateLambda([this] {
				LaunchURL(GitHub::README::Cloud);
			})
		)
	);
	
	MenuBuilder.EndSection();
}
