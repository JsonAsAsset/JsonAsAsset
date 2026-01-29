/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Modules/Toolbar/Toolbar.h"

#include "Utilities/Compatibility.h"
#include "Utilities/EngineUtilities.h"

#include "Modules/UI/StyleModule.h"
#include "Importers/Constructor/ImportReader.h"
#include "Modules/Metadata.h"
#include "Modules/Cloud/Cloud.h"
#include "Modules/Toolbar/Dropdowns/CloudToolsDropdownBuilder.h"
#include "Settings/Runtime.h"
#include "Modules/Toolbar/Dropdowns/GeneralDropdownBuilder.h"
#include "Modules/Toolbar/Dropdowns/DonateDropdownBuilder.h"
#include "Modules/Toolbar/Dropdowns/ParentDropdownBuilder.h"
#include "Modules/Toolbar/Dropdowns/ToolsDropdownBuilder.h"
#include "Modules/Toolbar/Dropdowns/VersioningDropdownBuilder.h"

void FJsonAsAssetToolbar::Register() {
#if ENGINE_UE5
	/* false: uses top toolbar. true: uses content browser toolbar */
	static bool bUseToolbar = true;
	
	UToolMenu* Menu;

	if (bUseToolbar) {
		Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	} else {
		Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.Toolbar");
	}

	FToolMenuSection& Section =
		bUseToolbar
		? Menu->FindOrAddSection(GJsonAsAssetName)
		: Menu->FindOrAddSection("New");

	/* Displays JsonAsAsset's icon along with the Version */
	FToolMenuEntry& ActionButton = Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		GJsonAsAssetName,
		
		FToolUIActionChoice(
			FUIAction(
				FExecuteAction::CreateStatic(&ImportAction),
				FCanExecuteAction(),
				FGetActionCheckState(),
				FIsActionButtonVisible::CreateStatic(&IsToolBarVisible)
			)
		),
		
		FText::FromString(FJMetadata::Version),
		
		FText::FromString("Execute JsonAsAsset"),
		
		FSlateIcon(FJsonAsAssetStyle::Get().GetStyleSetName(), FName("Toolbar.Icon")),
		
		EUserInterfaceActionType::Button
	));
	
	ActionButton.StyleNameOverride = "CalloutToolbar";

	/* Menu dropdown */
	const FToolMenuEntry MenuButton = Section.AddEntry(FToolMenuEntry::InitComboButton(
		"JsonAsAssetMenu",
		FUIAction(
			FExecuteAction(),
			FCanExecuteAction(),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(IsToolBarVisible)
		),
		FOnGetContent::CreateStatic(&CreateMenuDropdown),
		FText::FromString(GJsonAsAssetName.ToString()),
		FText::FromString(""),
		FSlateIcon(),
		true
	));

	AddCloudButtons(Section);
#endif
}

void FJsonAsAssetToolbar::AddCloudButtons(FToolMenuSection& Section)
{
#if ENGINE_UE5
	/* Adds the Cloud button to the toolbar */
	FToolMenuEntry& ActionButton = Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		"JsonAsAssetCloud",
		
		FToolUIActionChoice(
			FUIAction(
				FExecuteAction::CreateLambda([this] {
					UJsonAsAssetSettings* Settings = GetSettings();
					
					Settings->bEnableCloudServer = !Settings->bEnableCloudServer;
				}),
				FCanExecuteAction(),
				FGetActionCheckState(),
				FIsActionButtonVisible::CreateStatic(&IsToolBarVisible)
			)
		),
		
		TAttribute<FText>::CreateLambda([this] {
			const UJsonAsAssetSettings* Settings = GetSettings();
			
			return Settings->bEnableCloudServer
				? FText::FromString("On")
				: FText::FromString("Off");
		}),
		
		FText::FromString(""),
		
		FSlateIcon(FJsonAsAssetStyle::Get().GetStyleSetName(), FName("Toolbar.Cloud")),
		
		EUserInterfaceActionType::Button
	));
	
	ActionButton.StyleNameOverride = "CalloutToolbar";

	/* Menu dropdown */
	const FToolMenuEntry MenuButton = Section.AddEntry(FToolMenuEntry::InitComboButton(
		"JsonAsAssetCloudMenu",
		FUIAction(
			FExecuteAction(),
			FCanExecuteAction::CreateLambda([] {
				return GetSettings()->bEnableCloudServer;
			}),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(IsToolBarVisible)
		),
		FOnGetContent::CreateStatic(&CreateCloudMenuDropdown),
		FText::FromString(""),
		FText::FromString(""),
		FSlateIcon(),
		true
	));
#endif
}

#if ENGINE_UE4
void FJsonAsAssetToolbar::UE4Register(FToolBarBuilder& Builder) {
	Builder.AddToolBarButton(
		FUIAction(
			FExecuteAction::CreateStatic(&ImportAction),
			FCanExecuteAction(),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(IsToolBarVisible)
		),
		NAME_None,
		FText::FromString(FJMetadata::Version),
		FText::FromString("Execute JsonAsAsset"),
		FSlateIcon(FJsonAsAssetStyle::Get().GetStyleSetName(), FName("Toolbar.Icon"))
	);

	Builder.AddComboButton(
		FUIAction(
			FExecuteAction(),
			FCanExecuteAction(),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(IsToolBarVisible)
		),
		FOnGetContent::CreateStatic(&FJsonAsAssetToolbar::CreateMenuDropdown),
		FText::FromString(FJMetadata::Version),
		FText::FromString(""),
		FSlateIcon(FJsonAsAssetStyle::Get().GetStyleSetName(), FName("Toolbar.Icon")),
		true
	);
}

#endif

bool FJsonAsAssetToolbar::IsToolBarVisible() {
	bool bVisible = true;

	if (static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("Toolbar.Tools.FlippedVisibility"))) {
		if (CVar->GetInt() == 1) {
			bVisible = false;
		}
	}

	if (GEditor) {
		const TIndirectArray<FWorldContext>& WorldContextList = GEditor->GetWorldContexts();

		for (const FWorldContext& WorldContext : WorldContextList) {
			if (WorldContext.World() && WorldContext.World()->WorldType == EWorldType::PIE) {
				bVisible = false;
			}
		}
	}

	return bVisible;
}

bool FJsonAsAssetToolbar::IsFitToFunction() {
	const UJsonAsAssetSettings* Settings = GetSettings();

	/* Conditional Settings Checks */
	if (Settings->bEnableCloudServer) {
		if (!Cloud::Status::Check(Settings) || !Cloud::Update()) return false;
	}

	/* Update Runtime */
	GJsonAsAssetRuntime.Update();
	FJRedirects::Clear();

	return true;
}

void FJsonAsAssetToolbar::ImportAction() {
	if (!IsFitToFunction()) return;

	/* Dialog for a JSON File */
	TArray<FString> OutFileNames = OpenFileDialog("Select a JSON File", "JSON Files|*.json");
	if (OutFileNames.Num() == 0) {
		return;
	}

	for (FString& File : OutFileNames) {
		IImportReader::ImportReference(File);
	}
}

TSharedRef<SWidget> FJsonAsAssetToolbar::CreateMenuDropdown() {
	FMenuBuilder MenuBuilder(false, nullptr);

	TArray<TSharedRef<IParentDropdownBuilder>> Dropdowns = {
		MakeShared<IVersioningDropdownBuilder>(),
		MakeShared<IParentDropdownBuilder>(),
		MakeShared<IToolsDropdownBuilder>(),
		MakeShared<IGeneralDropdownBuilder>(),
		MakeShared<IDonateDropdownBuilder>()
	};

	for (const TSharedRef<IParentDropdownBuilder>& Dropdown : Dropdowns) {
		Dropdown->Build(MenuBuilder);
	}
	
	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> FJsonAsAssetToolbar::CreateCloudMenuDropdown() {
	FMenuBuilder MenuBuilder(false, nullptr);

	TArray<TSharedRef<IParentDropdownBuilder>> Dropdowns = {
		MakeShared<ICloudToolsDropdownBuilder>()
	};

	for (const TSharedRef<IParentDropdownBuilder>& Dropdown : Dropdowns) {
		Dropdown->Build(MenuBuilder);
	}
	
	return MenuBuilder.MakeWidget();
}
