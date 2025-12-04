/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Toolbar/Dropdowns/ParentDropdownBuilder.h"

#include "JsonAsAsset.h"
#include "Interfaces/IPluginManager.h"
#include "Utilities/Compatibility.h"
#include "Utilities/EngineUtilities.h"

void IParentDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(GJsonAsAssetName.ToString());
	
	MenuBuilder.BeginSection(
		"JsonAsAssetSection", 
		FText::FromString(Plugin->GetDescriptor().VersionName)
	);

	/* Start Of Section ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	MenuBuilder.AddMenuEntry(
		FText::FromString("Asset Types"),
		FText::FromString("List of supported assets for JsonAsAsset"),
		FSlateIcon(FAppStyle::Get().GetStyleSetName(), "LevelEditor.Tabs.Viewports"),
		FUIAction(
			FExecuteAction::CreateLambda([this]() {
				LaunchURL(GitHub::README::AssetTypes);
			})
		)
	);

	MenuBuilder.AddMenuEntry(
	FText::FromString("Documentation"),
	FText::FromString("View JsonAsAsset documentation"),
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
