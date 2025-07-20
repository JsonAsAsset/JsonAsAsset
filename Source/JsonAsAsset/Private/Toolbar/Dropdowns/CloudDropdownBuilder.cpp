/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Toolbar/Dropdowns/CloudDropdownBuilder.h"

#include "Modules/CloudModule.h"
#include "Utilities/EngineUtilities.h"

#include "Modules/Tools/AnimationData.h"
#include "Modules/Tools/ClearImportData.h"
#include "Modules/Tools/ConvexCollision.h"
#include "Modules/Tools/SkeletalMeshData.h"

void ICloudDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	UJsonAsAssetSettings* Settings = GetSettings();
	
	/* Cloud must be enabled, and if there is an action required, don't create Cloud's dropdown */
	if (!Settings->bEnableCloudServer || !UJsonAsAssetSettings::IsSetup(Settings) || !CloudModule::IsSetup(Settings)) {
		return;
	}
	
	MenuBuilder.BeginSection("JsonAsAssetSection", FText::FromString("Cloud"));

	MenuBuilder.AddMenuEntry(
		FText::FromString("Learn how to setup"),
		FText::FromString(""),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]() {
				const FString URL = "https://github.com/JsonAsAsset/JsonAsAsset?tab=readme-ov-file#4-cloud-server";
				FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
			})
		)
	);

	if (Settings->bEnableExperiments) {
		MenuBuilder.AddSeparator();
		MenuBuilder.AddSubMenu(
			FText::FromString("Fetch Tools"),
			FText::FromString("Extra tools to use with Cloud"),
			FNewMenuDelegate::CreateLambda([this](FMenuBuilder& InnerMenuBuilder) {
				InnerMenuBuilder.BeginSection("JsonAsAssetToolsSection", FText::FromString("Tools"));
				{
					InnerMenuBuilder.AddMenuEntry(
						FText::FromString("Import Static Mesh Properties from Cloud"),
						FText::FromString("Imports collision, properties and more using Cloud and applies it to the corresponding assets in the content browser folder."),
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

						FUIAction(
							FExecuteAction::CreateStatic(&FToolConvexCollision::Execute),
							FCanExecuteAction::CreateLambda([this] {
								return CloudModule::IsCloudRunning();
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
								return CloudModule::IsCloudRunning();
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
								return CloudModule::IsCloudRunning();
							})
						),
						NAME_None
					);
					
					InnerMenuBuilder.AddMenuEntry(
						FText::FromString("Clear Import Data"),
						FText::FromString(""),
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

						FUIAction(
							FExecuteAction::CreateStatic(&FToolClearImportData::Execute),
							FCanExecuteAction::CreateLambda([this] {
								return CloudModule::IsCloudRunning();
							})
						),
						NAME_None
					);
				}
				InnerMenuBuilder.EndSection();
			}),
			false
		);
	}
	
	MenuBuilder.EndSection();
}
