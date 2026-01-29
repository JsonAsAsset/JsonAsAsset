/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Modules/Toolbar/Dropdowns/GeneralDropdownBuilder.h"

#include "Utilities/Compatibility.h"
#include "Utilities/EngineUtilities.h"

void IGeneralDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	MenuBuilder.AddSeparator();
	
	MenuBuilder.AddMenuEntry(
		FText::FromString("Open Plugin Settings"),
		FText::FromString("Navigate to plugin settings"),
#if ENGINE_UE5
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Settings"),
#else
		FSlateIcon(FEditorStyle::GetStyleSetName(), "ProjectSettings.TabIcon"),
#endif
		FUIAction(
			FExecuteAction::CreateLambda([this]() {
				OpenPluginSettings();
			})
		),
		NAME_None
	);
}
