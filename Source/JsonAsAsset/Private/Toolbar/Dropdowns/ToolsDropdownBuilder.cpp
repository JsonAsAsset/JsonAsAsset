/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Toolbar/Dropdowns/ToolsDropdownBuilder.h"

#include "Importers/Constructor/Importer.h"
#include "Importers/Constructor/ImportReader.h"
#include "Utilities/EngineUtilities.h"

#include "Modules/Cloud/Tools/AnimationData.h"
#include "Modules/Tools/ClearImportData.h"
#include "Modules/Cloud/Tools/ConvexCollision.h"
#include "Modules/Cloud/Tools/SkeletalMeshData.h"

void IToolsDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	UJsonAsAssetSettings* Settings = GetSettings();
	
	if (!Settings->IsValid()) {
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
						FExecuteAction::CreateLambda([] {
							TToolClearImportData* Tool = new TToolClearImportData();
							Tool->Execute();
						})
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
									IImportReader::ImportReference(JsonPath);
								}
							}
						})
					),
					NAME_None
				);
			}
			InnerMenuBuilder.EndSection();

			if (Settings->bEnableCloudServer) {
				static TSkeletalMeshData* SkeletalMeshTool;

				if (SkeletalMeshTool == nullptr) {
					SkeletalMeshTool = new TSkeletalMeshData();
				}
				
				InnerMenuBuilder.BeginSection("JsonAsAssetCloudToolsSection", FText::FromString("Cloud"));
				
				InnerMenuBuilder.AddMenuEntry(
					FText::FromString("Import Static Mesh Properties"),
					FText::FromString("Imports collision, properties and more using Cloud and applies it to the corresponding assets in the content browser folder."),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

					FUIAction(
						FExecuteAction::CreateLambda([] {
							TToolConvexCollision* Tool = new TToolConvexCollision();
							Tool->Execute();
						}),
						FCanExecuteAction::CreateLambda([this] {
							return Cloud::Status::IsOpened();
						})
					),
					NAME_None
				);

				InnerMenuBuilder.AddMenuEntry(
					FText::FromString("Import Animation Data"),
					FText::FromString("Imports Animation Data using Cloud and applies it to the corresponding assets in the content browser folder."),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

					FUIAction(
						FExecuteAction::CreateLambda([] {
							TToolAnimationData* Tool = new TToolAnimationData();
							Tool->Execute();
						}),
						FCanExecuteAction::CreateLambda([this] {
							return Cloud::Status::IsOpened();
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
							if (SkeletalMeshTool != nullptr)
							{
								SkeletalMeshTool->Execute();
							}
						}),
						FCanExecuteAction::CreateLambda([this] {
							return Cloud::Status::IsOpened();
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
