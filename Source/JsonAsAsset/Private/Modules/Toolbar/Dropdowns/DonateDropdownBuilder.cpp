/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Modules/Toolbar/Dropdowns/DonateDropdownBuilder.h"

#include "JsonAsAsset.h"
#include "Modules/UI/StyleModule.h"
#include "Utilities/EngineUtilities.h"

void IDonateDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	MenuBuilder.BeginSection("JsonAsAssetSupportSection", FText::FromString("Support"));
	
	MenuBuilder.AddMenuEntry(
		FText::FromString("Become A Supporter"),
		FText::FromString("Help support JsonAsAsset's development"),
		FSlateIcon(FJsonAsAssetStyle::Get().GetStyleSetName(), FName("Toolbar.Heart")),
		FUIAction(
			FExecuteAction::CreateLambda([this] {
				LaunchURL(Donation::KO_FI);
			})
		)
	);
	
	MenuBuilder.EndSection();
}
