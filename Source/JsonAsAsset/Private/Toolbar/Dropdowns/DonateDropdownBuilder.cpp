/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Toolbar/Dropdowns/DonateDropdownBuilder.h"

#include "JsonAsAsset.h"
#include "Modules/UI/StyleModule.h"

void IDonateDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	MenuBuilder.AddMenuEntry(
		FText::FromString("Support Us"),
		FText::FromString(""),
		FSlateIcon(FJsonAsAssetStyle::Get().GetStyleSetName(), FName("Toolbar.Heart")),
		FUIAction(
			FExecuteAction::CreateLambda([this] {
				FPlatformProcess::LaunchURL(*Donation::KO_FI, nullptr, nullptr);
			})
		)
	);
}
