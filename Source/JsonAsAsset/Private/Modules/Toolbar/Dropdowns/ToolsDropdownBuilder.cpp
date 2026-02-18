/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Modules/Toolbar/Dropdowns/ToolsDropdownBuilder.h"

#include "Importers/Constructor/Importer.h"
#include "Importers/Constructor/ImportReader.h"
#include "Modules/Toolbar/Dropdowns/CloudToolsDropdownBuilder.h"
#include "Utilities/EngineUtilities.h"

#include "Modules/Tools/ClearImportData.h"

void IToolsDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	UJsonAsAssetSettings* Settings = GetSettings();
	
	MenuBuilder.AddSubMenu(
		FText::FromString("Asset Tools"),
		FText::FromString("Tools bundled"),
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
							for (FString Folder : OpenFolderDialog("Folder of JSON files")) {
								TArray<FString> JsonFiles;
								IFileManager::Get().FindFilesRecursive(
									JsonFiles,
									*Folder,
									TEXT("*.json"),
									true,
									true,
									false
								);

								for (FString& JsonPath : JsonFiles) {
									IImportReader::ImportReference(JsonPath);
								}
							}
						})
					),
					NAME_None
				);

				InnerMenuBuilder.EndSection();
			}

#if ENGINE_UE4
		if (Settings->EnableCloudServer) {
			TArray<TSharedRef<IParentDropdownBuilder>> Dropdowns = {
				MakeShared<ICloudToolsDropdownBuilder>()
			};

			for (const TSharedRef<IParentDropdownBuilder>& Dropdown : Dropdowns) {
				Dropdown->Build(InnerMenuBuilder);
			}
		}
#endif
		}),
		false,
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "DeveloperTools.MenuIcon")
	);
}
