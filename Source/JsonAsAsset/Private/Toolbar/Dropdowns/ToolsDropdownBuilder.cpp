/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Toolbar/Dropdowns/ToolsDropdownBuilder.h"

#include "Importers/Constructor/Importer.h"
#include "Modules/CloudModule.h"
#include "Utilities/EngineUtilities.h"

#include "Modules/Tools/AnimationData.h"
#include "Modules/Tools/ClearImportData.h"
#include "Modules/Tools/ConvexCollision.h"
#include "Modules/Tools/SkeletalMeshData.h"

void IToolsDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	UJsonAsAssetSettings* Settings = GetSettings();
	
	if (!UJsonAsAssetSettings::IsSetup(Settings)) {
		return;
	}
	
	MenuBuilder.AddSubMenu(
		FText::FromString("Tools"),
		FText::FromString("Tools bundled with JsonAsAsset"),
		FNewMenuDelegate::CreateLambda([this, Settings](FMenuBuilder& InnerMenuBuilder) {
			InnerMenuBuilder.BeginSection("JsonAsAssetToolsSection", FText::FromString("Tools"));
			{
				InnerMenuBuilder.AddMenuEntry(
					FText::FromString("Clear Import Data"),
					FText::FromString(""),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

					FUIAction(
						FExecuteAction::CreateStatic(&FToolClearImportData::Execute)
					),
					NAME_None
				);

				if (Settings->bEnableCloudServer) {
					InnerMenuBuilder.AddSeparator();
					
					InnerMenuBuilder.AddMenuEntry(
						FText::FromString("Import Static Mesh Properties from Cloud"),
						FText::FromString("Imports collision, properties and more using Cloud and applies it to the corresponding assets in the content browser folder."),
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

						FUIAction(
							FExecuteAction::CreateStatic(&FToolConvexCollision::Execute),
							FCanExecuteAction::CreateLambda([this] {
								return CloudModule::IsRunning();
							})
						),
						NAME_None
					);

					InnerMenuBuilder.AddMenuEntry(
						FText::FromString("Import Animation Data from Cloud"),
						FText::FromString("Imports animation data using Cloud and applies it to the corresponding assets in the content browser folder."),
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

						FUIAction(
							FExecuteAction::CreateStatic(&FToolAnimationData::Execute),
							FCanExecuteAction::CreateLambda([this] {
								return CloudModule::IsRunning();
							})
						),
						NAME_None
					);

					InnerMenuBuilder.AddMenuEntry(
						FText::FromString("Import Skeletal Mesh Data from Cloud"),
						FText::FromString("Imports skeletal mesh data using Cloud and applies it to the corresponding assets in the content browser folder."),
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

						FUIAction(
							FExecuteAction::CreateStatic(&FSkeletalMeshData::Execute),
							FCanExecuteAction::CreateLambda([this] {
								return CloudModule::IsRunning();
							})
						),
						NAME_None
					);
				}
			}
			InnerMenuBuilder.EndSection();
		}),
		false,
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "DeveloperTools.MenuIcon")
	);
}
