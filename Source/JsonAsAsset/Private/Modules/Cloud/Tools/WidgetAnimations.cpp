/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Modules/Cloud/Tools/WidgetAnimations.h"

#include "WidgetBlueprint.h"
#include "Utilities/EngineUtilities.h"

void TWidgetAnimations::Process(UObject* Object) {
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Object);
	if (!WidgetBlueprint) return;

	/* Empty all animations */
	WidgetBlueprint->Animations.Empty();

	FUObjectExportContainer Exports(SendToCloudForExports(GetAssetPath(Object)));
	FUObjectExport Export = Exports.FindByType(FString("WidgetBlueprintGeneratedClass"));

	auto Animations = Export.GetProperties()->GetStringField()
}
