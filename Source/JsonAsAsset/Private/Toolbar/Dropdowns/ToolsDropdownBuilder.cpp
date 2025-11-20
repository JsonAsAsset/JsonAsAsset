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
		FText::FromString("Asset Tools"),
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

				InnerMenuBuilder.AddMenuEntry(
					FText::FromString("Import Folder of JSON Files"),
					FText::FromString(""),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

					FUIAction(
						FExecuteAction::CreateLambda([] {
							for (FString Folder : OpenFolderDialog("Folder of JSON files"))
							{
								TArray<FString> JsonFiles;
								IFileManager::Get().FindFilesRecursive(
									JsonFiles,
									*Folder,
									TEXT("*.json"),
									true,
									true,
									false
								);

								for (FString& JsonPath : JsonFiles)
								{
									IImporter::ImportReference(JsonPath);
								}
							}
						})
					),
					NAME_None
				);
			}
			InnerMenuBuilder.EndSection();

			if (Settings->bEnableCloudServer) {
				InnerMenuBuilder.BeginSection("JsonAsAssetCloudToolsSection", FText::FromString("Cloud"));
				
				InnerMenuBuilder.AddMenuEntry(
					FText::FromString("Import Static Mesh Properties"),
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
					FText::FromString("Import Animation Data"),
					FText::FromString("Imports Animation Data using Cloud and applies it to the corresponding assets in the content browser folder."),
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
					FText::FromString("Import Skeletal Mesh Data"),
					FText::FromString("Imports Skeletal Mesh Data using Cloud and applies it to the corresponding assets in the content browser folder."),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

					FUIAction(
						FExecuteAction::CreateLambda([] {
							FSkeletalMeshData SkeletalMeshData = FSkeletalMeshData();
							SkeletalMeshData.Execute();
						}),
						FCanExecuteAction::CreateLambda([this] {
							return CloudModule::IsRunning();
						})
					),
					NAME_None
				);

				InnerMenuBuilder.EndSection();
			}
		}),
		false,
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "DeveloperTools.MenuIcon")
	);
}
