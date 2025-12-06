/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Toolbar/Toolbar.h"

#include "Utilities/Compatibility.h"
#include "Utilities/EngineUtilities.h"

#if ENGINE_UE5
#include "Modules/UI/StyleModule.h"
#endif

#include "Importers/Constructor/ImportReader.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/Cloud/Cloud.h"
#include "Settings/Runtime.h"
#include "Toolbar/Dropdowns/ActionRequiredDropdownBuilder.h"
#include "Toolbar/Dropdowns/GeneralDropdownBuilder.h"
#include "Toolbar/Dropdowns/CloudDropdownBuilder.h"
#include "Toolbar/Dropdowns/DonateDropdownBuilder.h"
#include "Toolbar/Dropdowns/ParentDropdownBuilder.h"
#include "Toolbar/Dropdowns/ToolsDropdownBuilder.h"
#include "Toolbar/Dropdowns/VersioningDropdownBuilder.h"

class FMessageLogModule;

/* ReSharper disable once CppMemberFunctionMayBeStatic */
void FJsonAsAssetToolbar::Register() {
#if ENGINE_UE5
	/* Get Plugin's VersionName, example: 1.0.0 */
    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(GJsonAsAssetName.ToString());
	const FString VersionName = Plugin->GetDescriptor().VersionName;

	/* Displays JsonAsAsset's icon along with the Version */
	FToolMenuEntry ActionButton = FToolMenuEntry::InitToolBarButton(
		GJsonAsAssetName,
		
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
				return FSlateIcon(FJsonAsAssetStyle::Get().GetStyleSetName(), FName("Toolbar.Icon"));
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
		FText::FromString(GJsonAsAssetName.ToString()),
		FText::FromString("Open JsonAsAsset's Menu"),
		FSlateIcon(),
		true
	);

	/* Extend the menu */
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	FToolMenuSection& Section = Menu->FindOrAddSection(GJsonAsAssetName);

	Section.AddEntry(ActionButton);
	Section.AddEntry(MenuButton);
#endif
}

#if ENGINE_UE4
void FJsonAsAssetToolbar::UE4Register(FToolBarBuilder& Builder) {
    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(GJsonAsAssetName.ToString());
	
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
		FSlateIcon(FJsonAsAssetStyle::Get().GetStyleSetName(), FName("Toolbar.Icon"))
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

/* ReSharper disable once CppMemberFunctionMayBeStatic */
bool FJsonAsAssetToolbar::IsActionEnabled() const {
	return GetSettings()->IsValid();
}

/* ReSharper disable once CppMemberFunctionMayBeStatic */
FText FJsonAsAssetToolbar::GetTooltipText() const {
	return !GetSettings()->IsValid()
		? FText::FromString("The button is disabled due to your settings being improperly setup. Please modify your settings to execute JsonAsAsset.")
		: FText::FromString("Execute JsonAsAsset");
}

/* ReSharper disable once CppMemberFunctionMayBeStatic */
void FJsonAsAssetToolbar::ImportAction() {
	const UJsonAsAssetSettings* Settings = GetSettings();

	/* Conditional Settings Checks */
	if (Settings->bEnableCloudServer) {
		if (!Cloud::Status::Check(Settings)) return;
		Cloud::Update();
	}

	/* Update Runtime */
	GJsonAsAssetRuntime.Update();
	FJRedirects::Clear();

	/* Dialog for a JSON File */
	TArray<FString> OutFileNames = OpenFileDialog("Select a JSON File", "JSON Files|*.json");
	if (OutFileNames.Num() == 0) {
		return;
	}

	for (FString& File : OutFileNames) {
		EmptyMessageLog();

		IImportReader::ImportReference(File);
	}
}

/* ReSharper disable once CppMemberFunctionMayBeStatic */
TSharedRef<SWidget> FJsonAsAssetToolbar::CreateMenuDropdown() {
	FMenuBuilder MenuBuilder(false, nullptr);

	TArray<TSharedRef<IParentDropdownBuilder>> Dropdowns = {
		MakeShared<IVersioningDropdownBuilder>(),
		MakeShared<IParentDropdownBuilder>(),
		MakeShared<IToolsDropdownBuilder>(),
		MakeShared<IActionRequiredDropdownBuilder>(),
		MakeShared<ICloudDropdownBuilder>(),
		MakeShared<IGeneralDropdownBuilder>(),
		MakeShared<IDonateDropdownBuilder>()
	};

	for (const TSharedRef<IParentDropdownBuilder>& Dropdown : Dropdowns) {
		Dropdown->Build(MenuBuilder);
	}
	
	return MenuBuilder.MakeWidget();
}