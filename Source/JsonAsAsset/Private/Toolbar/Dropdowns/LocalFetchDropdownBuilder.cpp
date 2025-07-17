/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Toolbar/Dropdowns/LocalFetchDropdownBuilder.h"

#include "Modules/LocalFetchModule.h"
#include "Utilities/EngineUtilities.h"

#include "Modules/Tools/AnimationData.h"
#include "Modules/Tools/ClearImportData.h"
#include "Modules/Tools/ConvexCollision.h"
#include "Modules/Tools/SkeletalMeshData.h"

void ILocalFetchDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	UJsonAsAssetSettings* Settings = GetSettings();
	
	/* Local Fetch must be enabled, and if there is an action required, don't create Local Fetch's dropdown */
	if (!Settings->bEnableLocalFetch || !UJsonAsAssetSettings::IsSetup(Settings) || !LocalFetchModule::IsSetup(Settings)) {
		return;
	}
	
	MenuBuilder.BeginSection("JsonAsAssetSection", FText::FromString("Local Fetch"));
	MenuBuilder.AddMenuEntry(
		FText::FromString("Asset Types"),
		FText::FromString("List of supported classes that can be locally fetched using the API"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]() {
				const FString URL = "https://github.com/JsonAsAsset/JsonAsAsset?tab=readme-ov-file#4-local-fetch";
				FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
			})
		)
	);

	MenuBuilder.AddSubMenu(
		FText::FromString("Command-line Application"),
		FText::FromString(""),
		FNewMenuDelegate::CreateLambda([this](FMenuBuilder& InnerMenuBuilder) {
			InnerMenuBuilder.BeginSection("JsonAsAssetSection", FText::FromString("Local Fetch (.EXE)"));
			{
				if (LocalFetchModule::IsLocalFetchRunning()) {
					InnerMenuBuilder.AddMenuEntry(
						FText::FromString("Restart"),
						FText::FromString("Restarts the Local Fetch API"),
#if ENGINE_UE5
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "Blueprint.CompileStatus.Background", NAME_None),
#else
						FSlateIcon(FEditorStyle::GetStyleSetName(), "MainFrame.CreditsUnrealEd"),
#endif
						FUIAction(
							FExecuteAction::CreateLambda([this]() {
								LocalFetchModule::CloseLocalFetch();
								LocalFetchModule::LaunchLocalFetch();
							}),
							FCanExecuteAction::CreateLambda([this]() {
								return LocalFetchModule::IsLocalFetchRunning();
							})
						)
					);

					InnerMenuBuilder.AddMenuEntry(
						FText::FromString("Shutdown"),
						FText::FromString("Shutdown the Local Fetch API"),
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.RemoveLayout"),
						FUIAction(
							FExecuteAction::CreateLambda([this]() {
								LocalFetchModule::CloseLocalFetch();
							}),
							FCanExecuteAction::CreateLambda([this]() {
								return LocalFetchModule::IsLocalFetchRunning();
							})
						)
					);
				} else {
					InnerMenuBuilder.AddMenuEntry(
						FText::FromString("Launch"),
						FText::FromString("Launches the Local Fetch API"),
#if ENGINE_UE5
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "Blueprint.CompileStatus.Background", NAME_None),
#else
						FSlateIcon(FEditorStyle::GetStyleSetName(), "MainFrame.CreditsUnrealEd"),
#endif
						FUIAction(
							FExecuteAction::CreateLambda([this]() {
								const TSharedPtr<SNotificationItem> NotificationItem = LocalFetchNotification.Pin();

								RemoveNotification(NotificationItem);

								LocalFetchModule::LaunchLocalFetch();
							}),
							FCanExecuteAction::CreateLambda([this]() {
								return !LocalFetchModule::IsLocalFetchRunning();
							})
						)
					);
				}
			}
			InnerMenuBuilder.EndSection();
		}),
		false,
		FSlateIcon()
	);

	if (Settings->bEnableExperiments) {
		MenuBuilder.AddSeparator();
		MenuBuilder.AddSubMenu(
			FText::FromString("Fetch Tools"),
			FText::FromString("Extra tools to use with Local Fetch"),
			FNewMenuDelegate::CreateLambda([this](FMenuBuilder& InnerMenuBuilder) {
				InnerMenuBuilder.BeginSection("JsonAsAssetToolsSection", FText::FromString("Tools"));
				{
					InnerMenuBuilder.AddMenuEntry(
						FText::FromString("Import Static Mesh Properties from Local Fetch"),
						FText::FromString("Imports collision, properties and more using Local Fetch and applies it to the corresponding assets in the content browser folder."),
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

						FUIAction(
							FExecuteAction::CreateStatic(&FToolConvexCollision::Execute),
							FCanExecuteAction::CreateLambda([this] {
								return LocalFetchModule::IsLocalFetchRunning();
							})
						),
						NAME_None
					);

					InnerMenuBuilder.AddMenuEntry(
						FText::FromString("Import Animation Data from Local Fetch"),
						FText::FromString("Imports animation data using Local Fetch and applies it to the corresponding assets in the content browser folder."),
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

						FUIAction(
							FExecuteAction::CreateStatic(&FToolAnimationData::Execute),
							FCanExecuteAction::CreateLambda([this] {
								return LocalFetchModule::IsLocalFetchRunning();
							})
						),
						NAME_None
					);

					InnerMenuBuilder.AddMenuEntry(
						FText::FromString("Import Skeletal Mesh Data from Local Fetch"),
						FText::FromString("Imports skeletal mesh data using Local Fetch and applies it to the corresponding assets in the content browser folder."),
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

						FUIAction(
							FExecuteAction::CreateStatic(&FSkeletalMeshData::Execute),
							FCanExecuteAction::CreateLambda([this] {
								return LocalFetchModule::IsLocalFetchRunning();
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
								return LocalFetchModule::IsLocalFetchRunning();
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
