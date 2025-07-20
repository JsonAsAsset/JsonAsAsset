/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Toolbar/Dropdowns/ActionRequiredDropdownBuilder.h"

#include "ISettingsModule.h"
#include "Modules/CloudModule.h"
#include "Utilities/EngineUtilities.h"

void IActionRequiredDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	UJsonAsAssetSettings* Settings = GetSettings();

	if (!UJsonAsAssetSettings::IsSetup(Settings) || !CloudModule::IsSetup(Settings)) {
		MenuBuilder.BeginSection("JsonAsAssetActionRequired", FText::FromString("Action Required"));
		
		TArray<FString> RequiredActions;

		/* Export Directory Missing */
		if (!UJsonAsAssetSettings::IsSetup(Settings)) {
			RequiredActions.Add("Missing Export Directory");
		}
		
		if (Settings->bEnableCloudServer) {
			TArray<FString> CloudRequiredActions;
			
			if (!CloudModule::IsSetup(Settings, CloudRequiredActions)) {
				for (FString RequiredAction : CloudRequiredActions) {
					RequiredActions.Add(RequiredAction);
				}
			}
		}

		for (FString Action : RequiredActions) {
			MenuBuilder.AddMenuEntry(
				FText::FromString(Action),
				FText::FromString("Open JsonAsAsset Settings"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.WarningWithColor"),
				FUIAction(
					FExecuteAction::CreateLambda([this]() {
						/* Send user to JsonAsAsset's settings */
						FModuleManager::LoadModuleChecked<ISettingsModule>("Settings")
							.ShowViewer("Editor", "Plugins", "JsonAsAsset");
					})
				)
			);
		}
		MenuBuilder.EndSection();
	}
}
