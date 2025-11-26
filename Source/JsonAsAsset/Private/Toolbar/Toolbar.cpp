/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Toolbar/Toolbar.h"

#include "Utilities/Compatibility.h"
#include "Utilities/EngineUtilities.h"

#include "Importers/Constructor/Importer.h"

#if ENGINE_UE5
#include "Modules/UI/StyleModule.h"
#endif

#include "Interfaces/IPluginManager.h"
#include "Modules/Cloud/Cloud.h"
#include "Modules/Cloud/CloudStatus.h"
#include "Modules/UI/StyleModule.h"
#include "Toolbar/Dropdowns/ActionRequiredDropdownBuilder.h"
#include "Toolbar/Dropdowns/GeneralDropdownBuilder.h"
#include "Toolbar/Dropdowns/CloudDropdownBuilder.h"
#include "Toolbar/Dropdowns/ParentDropdownBuilder.h"
#include "Toolbar/Dropdowns/ToolsDropdownBuilder.h"
#include "Toolbar/Dropdowns/VersioningDropdownBuilder.h"

class FMessageLogModule;

/* ReSharper disable once CppMemberFunctionMayBeStatic */
void FJsonAsAssetToolbar::Register() {
#if ENGINE_UE5
	/* Get Plugin's VersionName, example: 1.0.0 */
    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("JsonAsAsset");
	const FString VersionName = Plugin->GetDescriptor().VersionName;

	/* Displays JsonAsAsset's icon along with the Version */
	FToolMenuEntry ActionButton = FToolMenuEntry::InitToolBarButton(
		FName("JsonAsAsset"),
		
		FToolUIActionChoice(
			FUIAction(
				FExecuteAction::CreateRaw(this, &FJsonAsAssetToolbar::ImportAction),
				FCanExecuteAction::CreateRaw(this, &FJsonAsAssetToolbar::IsActionEnabled),
				FGetActionCheckState(),
				FIsActionButtonVisible::CreateStatic(IsToolBarVisible)
			)
		),
		
		FText::FromString(VersionName),
		
		TAttribute<FText>::CreateRaw(this, &FJsonAsAssetToolbar::GetTooltipText),
		
		TAttribute<FSlateIcon>::Create(
			TAttribute<FSlateIcon>::FGetter::CreateLambda([this]() -> FSlateIcon {
				return FSlateIcon(FJsonAsAssetStyle::Get().GetStyleSetName(), FName("JsonAsAsset.Toolbar.Icon"));
			})
		),
		
		EUserInterfaceActionType::Button
	);
	
	ActionButton.StyleNameOverride = "CalloutToolbar";

	/* Menu dropdown */
	const FToolMenuEntry MenuButton = FToolMenuEntry::InitComboButton(
		"JsonAsAssetMenu",
		FUIAction(
			FExecuteAction(),
			FCanExecuteAction(),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(IsToolBarVisible)
		),
		FOnGetContent::CreateRaw(this, &FJsonAsAssetToolbar::CreateMenuDropdown),
		FText::FromString("JsonAsAsset"),
		FText::FromString("Open JsonAsAsset's Menu"),
		FSlateIcon(),
		true
	);

	/* Extend the menu */
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	FToolMenuSection& Section = Menu->FindOrAddSection("JsonAsAsset");

	Section.AddEntry(ActionButton);
	Section.AddEntry(MenuButton);
#endif
}

#if ENGINE_UE4
void FJsonAsAssetToolbar::UE4Register(FToolBarBuilder& Builder) {
    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("JsonAsAsset");
	
	Builder.AddToolBarButton(
		FUIAction(
			FExecuteAction::CreateRaw(this, &FJsonAsAssetToolbar::ImportAction),
			FCanExecuteAction::CreateRaw(this, &FJsonAsAssetToolbar::IsActionEnabled),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(IsToolBarVisible)
		),
		NAME_None,
		FText::FromString(Plugin->GetDescriptor().VersionName),
		FText::FromString("Execute JsonAsAsset"),
		FSlateIcon(FJsonAsAssetStyle::Get().GetStyleSetName(), FName("JsonAsAsset.Toolbar.Icon"))
	);

	Builder.AddComboButton(
		FUIAction(
			FExecuteAction(),
			FCanExecuteAction(),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(IsToolBarVisible)
		),
		FOnGetContent::CreateRaw(this, &FJsonAsAssetToolbar::CreateMenuDropdown),
		FText::FromString(Plugin->GetDescriptor().VersionName),
		FText::FromString("Open JsonAsAsset's Menu"),
		FSlateIcon(FJsonAsAssetStyle::Get().GetStyleSetName(), FName("JsonAsAsset.Toolbar.Icon")),
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

/* ReSharper disable once CppMemberFunctionMayBeStatic */
bool FJsonAsAssetToolbar::IsActionEnabled() const {
	return UJsonAsAssetSettings::IsSetup(GetSettings());
}

/* ReSharper disable once CppMemberFunctionMayBeStatic */
FText FJsonAsAssetToolbar::GetTooltipText() const {
	return !UJsonAsAssetSettings::IsSetup(GetSettings())
		? FText::FromString("The button is disabled due to your settings being improperly setup. Please modify your settings to execute JsonAsAsset.")
		: FText::FromString("Execute JsonAsAsset");
}

/* ReSharper disable once CppMemberFunctionMayBeStatic */
void FJsonAsAssetToolbar::ImportAction() {
	UJsonAsAssetSettings* Settings = GetSettings();

	/* Conditional Settings Checks */
	if (!UJsonAsAssetSettings::EnsureExportDirectoryIsValid(Settings)) return;
	if (!Cloud::Status::Check(Settings)) return;
	Cloud::Update();

	/* Dialog for a JSON File */
	TArray<FString> OutFileNames = OpenFileDialog("Select a JSON File", "JSON Files|*.json");
	if (OutFileNames.Num() == 0) {
		return;
	}

	for (FString& File : OutFileNames) {
		EmptyMessageLog();

		IImporter* Importer = new IImporter();
		Importer->ImportReference(File);
	}
}

/* ReSharper disable once CppMemberFunctionMayBeStatic */
TSharedRef<SWidget> FJsonAsAssetToolbar::CreateMenuDropdown() {
	FMenuBuilder MenuBuilder(false, nullptr);

	TArray<TSharedRef<IParentDropdownBuilder>> Dropdowns = {
		MakeShared<IParentDropdownBuilder>(),
		MakeShared<IToolsDropdownBuilder>(),
		MakeShared<IActionRequiredDropdownBuilder>(),
		MakeShared<ICloudDropdownBuilder>(),
		MakeShared<IGeneralDropdownBuilder>(),
		MakeShared<IVersioningDropdownBuilder>()
	};

	for (const TSharedRef<IParentDropdownBuilder>& Dropdown : Dropdowns) {
		Dropdown->Build(MenuBuilder);
	}
	
	return MenuBuilder.MakeWidget();
}