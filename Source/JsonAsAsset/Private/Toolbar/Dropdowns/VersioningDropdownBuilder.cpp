/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Toolbar/Dropdowns/VersioningDropdownBuilder.h"

#include "Modules/Versioning.h"

void IVersioningDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	if (!GJsonAsAssetVersioning.bIsValid) {
		return;
	}
	
	MenuBuilder.BeginSection("JsonAsAssetVersioningSection", FText::FromString("Versioning"));
	
	FText Text, Tooltip;
	FSlateIcon Icon =
#if ENGINE_UE5
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Blueprint.CompileStatus.Background", NAME_None);
#else
		FSlateIcon(FEditorStyle::GetStyleSetName(), "MainFrame.CreditsUnrealEd");
#endif

	/* A new release is available */
	if (GJsonAsAssetVersioning.bNewVersionAvailable) {
		Text = FText::FromString("New Version Available");
		
		Tooltip = FText::FromString("Update your installation to version " + GJsonAsAssetVersioning.VersionName + " of JsonAsAsset");

		Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "Cascade.AddLODBeforeCurrent.Small");
	} else if (GJsonAsAssetVersioning.bFutureVersion) {
		Text = FText::FromString("Developmental Build");
		
		Tooltip = FText::FromString("You are currently running a developmental build of JsonAsAsset");
		
	} else {
		Text = FText::FromString("Latest Version");
		
		Tooltip = FText::FromString("You are currently using the latest version of JsonAsAsset");
	}

	MenuBuilder.AddMenuEntry(
		Text,
		Tooltip,
		Icon,
		FUIAction(
			FExecuteAction::CreateLambda([this]() {
				if (GJsonAsAssetVersioning.bNewVersionAvailable) {
					FPlatformProcess::LaunchURL(*GJsonAsAssetVersioning.HTMLUrl, nullptr, nullptr);
				}
			})
		),
		NAME_None
	);
	
	MenuBuilder.EndSection();
}
