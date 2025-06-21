/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Toolbar/Dropdowns/ParentDropdownBuilder.h"

#include "Importers/Constructor/Importer.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateIconFinder.h"
#include "Utilities/Compatibility.h"

void IParentDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("JsonAsAsset");
	
	MenuBuilder.BeginSection(
		"JsonAsAssetSection", 
		FText::FromString("JsonAsAsset v" + Plugin->GetDescriptor().VersionName)
	);

	/* Start Of Section ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	MenuBuilder.AddSubMenu(
		FText::FromString("Asset Types"),
		FText::FromString("List of supported assets for JsonAsAsset"),
		FNewMenuDelegate::CreateLambda([this](FMenuBuilder& InnerMenuBuilder) {
			DisplaySupportedAssetsMenu(InnerMenuBuilder);
		}),
		false,
		FSlateIcon(FAppStyle::Get().GetStyleSetName(), "LevelEditor.Tabs.Viewports")
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

void IParentDropdownBuilder::DisplaySupportedAssetsMenu(FMenuBuilder& InnerMenuBuilder, const bool IsLocalFetch) {
	TMap<FString, TArray<FString>> CategoriesAndTypes = ImporterTemplatedTypes;
	
	/* Add asset types from the factory registry */
	for (auto Pair : IImporter::GetFactoryRegistry()) {
		if (CategoriesAndTypes.Find(Pair.Value.Category)) {
			CategoriesAndTypes[Pair.Value.Category].Append(Pair.Key);
		} else {
			const UJsonAsAssetSettings* SettingsReferenced = GetDefault<UJsonAsAssetSettings>();

			if (!SettingsReferenced->bEnableExperiments) {
				bool IsExperimentalAssetType = false;

				for (FString ExperimentalAssetType : ExperimentalAssetTypes) {
					if (Pair.Key.Contains(ExperimentalAssetType)) {
						IsExperimentalAssetType = true;
					}
				}

				if (IsExperimentalAssetType) {
					continue;
				}
			}

			CategoriesAndTypes.Add(Pair.Value.Category, Pair.Key);
		}
	}

	/* Sort categories by alphabetical order */
	TArray<TPair<FString, TArray<FString>>> SortedCategoryPairs;
	for (const TPair<FString, TArray<FString>>& Pair : CategoriesAndTypes) {
		SortedCategoryPairs.Add(Pair);
	}

	SortedCategoryPairs.Sort([](const TPair<FString, TArray<FString>>& A, const TPair<FString, TArray<FString>>& B) {
		return A.Key < B.Key;
	});

	/* Sort asset types by alphabetical order */
	for (TPair<FString, TArray<FString>>& CategoryPair : CategoriesAndTypes) {
		CategoryPair.Value.Sort([](const FString& A, const FString& B) {
			return A < B;
		});
	}

	/* Go through each pair and add the entries */
	for (const TPair<FString, TArray<FString>>& Pair : SortedCategoryPairs) {
		FString Category = Pair.Key;
		TArray<FString> AssetTypes = Pair.Value;
					
		InnerMenuBuilder.BeginSection(FName(*Category), FText::FromString(Category));

		for (const FString& AssetType : AssetTypes) {
			if (IsLocalFetch) {
				if (!IImporter::CanImportWithLocalFetch(AssetType)) {
					continue;
				}
			}
			
			const UClass* Class = FindObject<UClass>(nullptr, *("/Script/Engine." + AssetType));
			FText Description = Class ? Class->GetToolTipText() : FText::FromString(AssetType);

			InnerMenuBuilder.AddMenuEntry(
				FText::FromString(AssetType),
				Description,
				FSlateIconFinder::FindCustomIconForClass(Class, TEXT("ClassThumbnail")),
				FUIAction(
					FExecuteAction::CreateLambda([] {
						return;
					})
				)
			);
		}

		InnerMenuBuilder.EndSection();
	}
}
