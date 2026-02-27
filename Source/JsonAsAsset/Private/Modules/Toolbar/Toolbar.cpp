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

#if PLATFORM_WINDOWS
static TWeakPtr<SNotificationItem> WaitingForCloud;
#endif

void UJsonAsAssetToolbar::Register() {
#if ENGINE_UE5
	/* false: uses top toolbar. true: uses content browser toolbar */
	static bool UseToolbar = false;
	
	UToolMenu* Menu;

	if (UseToolbar) {
		Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	} else {
		Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.Toolbar");
	}

	FToolMenuSection& Section =
		UseToolbar
		? Menu->FindOrAddSection(GJsonAsAssetName)
		: Menu->FindOrAddSection("New");

	/* Displays JsonAsAsset's icon along with the Version */
	FToolMenuEntry& ActionButton = Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		GJsonAsAssetName,
		
		FToolUIActionChoice(
			FUIAction(
				FExecuteAction::CreateUObject(this, &UJsonAsAssetToolbar::ImportAction),
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

void UJsonAsAssetToolbar::AddCloudButtons(FToolMenuSection& Section) {
#if ENGINE_UE5
	/* Adds the Cloud button to the toolbar */
	FToolMenuEntry& ActionButton = Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		"JsonAsAssetCloud",
		FToolUIActionChoice(
			FUIAction(
				FExecuteAction::CreateLambda([this] {
					UJsonAsAssetSettings* Settings = GetSettings();
					
					Settings->EnableCloudServer = !Settings->EnableCloudServer;
					SavePluginSettings(Settings);
				}),
				FCanExecuteAction(),
				FGetActionCheckState(),
				FIsActionButtonVisible::CreateStatic(&IsToolBarVisible)
			)
		),
		TAttribute<FText>::CreateLambda([this] {
			const UJsonAsAssetSettings* Settings = GetSettings();
			
			return Settings->EnableCloudServer ? FText::FromString("On") : FText::FromString("Off");
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
				return GetSettings()->EnableCloudServer;
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
void UJsonAsAssetToolbar::UE4Register(FToolBarBuilder& Builder) {
	Builder.AddToolBarButton(
		FUIAction(
			FExecuteAction::CreateUObject(this, &UJsonAsAssetToolbar::ImportAction),
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
		FOnGetContent::CreateStatic(&UJsonAsAssetToolbar::CreateMenuDropdown),
		FText::FromString(FJMetadata::Version),
		FText::FromString(""),
		FSlateIcon(FJsonAsAssetStyle::Get().GetStyleSetName(), FName("Toolbar.Icon")),
		true
	);
}

#endif

bool UJsonAsAssetToolbar::IsToolBarVisible() {
	bool Visible = true;

	if (static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("Toolbar.Tools.FlippedVisibility"))) {
		if (CVar->GetInt() == 1) {
			Visible = false;
		}
	}

	if (GEditor) {
		for (const FWorldContext& WorldContext : GEditor->GetWorldContexts()) {
			if (WorldContext.World() && WorldContext.World()->WorldType == EWorldType::PIE) {
				Visible = false;
			}
		}
	}

	return Visible;
}

void UJsonAsAssetToolbar::WaitForCloudTimerCallback() {
	if (WaitingForCloud.IsValid()) {
		CloudDotCount = (CloudDotCount + 1) % 4;

		FString Dots;
		for (int32 i = 0; i < CloudDotCount; ++i) {
			Dots += TEXT(".");
		}

		WaitingForCloud.Pin()->SetText(
			FText::FromString(FString::Printf(TEXT("Establishing Cloud%s"), *Dots))
		);
	}
	
	if (!GetSettings()->EnableCloudServer || !Cloud::Status::IsOpened()) {
		CancelWaitForCloudTimer();
		
		return;
	}

	Cloud::Status::IsReady([this](const bool bReady) {
		if (!bReady) {
			return;
		}

		CancelWaitForCloudTimer();
		ImportAction();
	});
}

void UJsonAsAssetToolbar::CancelWaitForCloudTimer() {
	RemoveNotification(WaitingForCloud);
	GEditor->GetTimerManager()->ClearTimer(WaitForCloudTimer);
}

void UJsonAsAssetToolbar::IsFitToFunction(TFunction<void(bool)> OnResponse) {
	const UJsonAsAssetSettings* Settings = GetSettings();

	if (!Settings->EnableCloudServer) {
		OnResponse(true);
		
		return;
	}

	Cloud::Status::Check(Settings,[this, OnResponse](const bool bStatusOk) {
		if (!bStatusOk) {
			OnResponse(false);
			
			return;
		}

		Cloud::Update([OnResponse](const bool bUpdated) {
			OnResponse(bUpdated);
		});
	});
}

void UJsonAsAssetToolbar::ImportAction() {
	if (WaitingForCloud.IsValid()) return;
	
	IsFitToFunction([this](const bool bAllowed) {
		if (!bAllowed) {
			HandleCloudWaiting();
			
			return;
		}

		Import();
	});
}

void UJsonAsAssetToolbar::Import() {
	/* Update Runtime */
	GJsonAsAssetRuntime.Update();
	FJRedirects::Clear();

	CancelWaitForCloudTimer();

	/* Dialog for a JSON File */
	TArray<FString> OutFileNames = OpenFileDialog("Select a JSON File", "JSON Files|*.json");
	if (OutFileNames.Num() == 0) {
		return;
	}

	for (FString& File : OutFileNames) {
		IImportReader::ImportReference(File);
	}
}

void UJsonAsAssetToolbar::HandleCloudWaiting() {
	if (!Cloud::Status::ShouldWaitUntilInitialized(GetSettings()) || WaitingForCloud.IsValid()) return;
	
	WaitingForCloud =
		AppendNotificationWithHandler(
			FText::FromString("Establishing Cloud"),
			FText::FromString(""),
			999.0f,
			FJsonAsAssetStyle::Get().GetBrush("Toolbar.Icon"),
			SNotificationItem::CS_Pending,
			false,
			0.0f);

	GEditor->GetTimerManager()->SetTimer(
		WaitForCloudTimer,
		FTimerDelegate::CreateUObject(this, &UJsonAsAssetToolbar::WaitForCloudTimerCallback),
		0.2f,
		true);
}

TSharedRef<SWidget> UJsonAsAssetToolbar::CreateMenuDropdown() {
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

TSharedRef<SWidget> UJsonAsAssetToolbar::CreateCloudMenuDropdown() {
	FMenuBuilder MenuBuilder(false, nullptr);

	TArray<TSharedRef<IParentDropdownBuilder>> Dropdowns = {
		MakeShared<ICloudToolsDropdownBuilder>()
	};

	for (const TSharedRef<IParentDropdownBuilder>& Dropdown : Dropdowns) {
		Dropdown->Build(MenuBuilder);
	}
	
	return MenuBuilder.MakeWidget();
}
