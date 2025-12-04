/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Toolbar/Dropdowns/ActionRequiredDropdownBuilder.h"

#include "Utilities/EngineUtilities.h"

void IActionRequiredDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	const UJsonAsAssetSettings* Settings = GetSettings();

	TArray<FString> RequiredActions;
	
	if (!Settings->IsValid(RequiredActions)) {
		MenuBuilder.BeginSection("JsonAsAssetActionRequired", FText::FromString("Action Required"));

		for (FString Action : RequiredActions) {
			MenuBuilder.AddMenuEntry(
				FText::FromString(Action),
				FText::FromString("Navigate to the JsonAsAsset plugin settings"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.ErrorWithColor"),
				FUIAction(
					FExecuteAction::CreateLambda([this]() {
						OpenPluginSettings();
					})
				)
			);
		}
		
		MenuBuilder.EndSection();
	}
}
