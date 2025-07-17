/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Toolbar/Dropdowns/ParentDropdownBuilder.h"

#include "Interfaces/IPluginManager.h"
#include "Utilities/Compatibility.h"

void IParentDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("JsonAsAsset");
	
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
				const FString URL = "https://github.com/JsonAsAsset/JsonAsAsset?tab=readme-ov-file#11-supported-asset-types";
				FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
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
				const FString URL = "https://github.com/JsonAsAsset/JsonAsAsset";
				FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
			})
		),
		NAME_None
	);
	/* End Of Section ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

	MenuBuilder.EndSection();
}