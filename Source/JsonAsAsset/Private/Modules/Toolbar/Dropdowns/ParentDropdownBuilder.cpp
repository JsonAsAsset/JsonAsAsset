/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Modules/Toolbar/Dropdowns/ParentDropdownBuilder.h"

#include "JsonAsAsset.h"
#include "Modules/Metadata.h"
#include "Utilities/Compatibility.h"
#include "Utilities/EngineUtilities.h"

void IParentDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	MenuBuilder.BeginSection(
		"JsonAsAssetSection", 
		FText::FromString(FJMetadata::Version)
	);

	/* Start Of Section ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	MenuBuilder.AddMenuEntry(
	FText::FromString("Documentation"),
	FText::FromString("View documentation"),
#if ENGINE_UE5
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Documentation"),
#else
		FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.BrowseDocumentation"),
#endif
		FUIAction(
			FExecuteAction::CreateLambda([this] {
				LaunchURL(GitHub::URL);
			})
		),
		NAME_None
	);
	/* End Of Section ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

	MenuBuilder.EndSection();
}
