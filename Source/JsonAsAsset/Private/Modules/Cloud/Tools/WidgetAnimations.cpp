/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Modules/Cloud/Tools/WidgetAnimations.h"
#include "WidgetBlueprint.h"
#include "Animation/WidgetAnimation.h"
#include "Utilities/EngineUtilities.h"

void TWidgetAnimations::Process(UObject* Object) {
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Object);
	if (!WidgetBlueprint) return;

	/* Empty all animations */
	for (UObject* Animation : WidgetBlueprint->Animations) {
		MoveToTransientPackageAndRename(Animation);
	}
	
	WidgetBlueprint->Animations.Empty();

	UWidgetBlueprintGeneratedClass* GeneratedClass = Cast<UWidgetBlueprintGeneratedClass>(WidgetBlueprint->GeneratedClass);
	for (UObject* AnimationObject : GeneratedClass->Animations) {
		MoveToTransientPackageAndRename(AnimationObject);
	}

	FUObjectExportContainer Exports(SendToCloudForExports(GetAssetPath(Object)));
	auto Export = Exports.FindByType(FString("WidgetBlueprintGeneratedClass"));

	auto Animations = Export.GetPropertiesNew().GetArray("Animations");

	GetObjectSerializer()->WhitelistedTypes.Add("MovieScene");
	GetObjectSerializer()->WhitelistedTypes.Add("WidgetAnimation");

	Initialize(Export, Exports);
	DeserializeExports(WidgetBlueprint, true);

	for (FUObjectExport AnimationExport : GetPropertySerializer()->ExportsContainer) {
		if (AnimationExport.Object) {
			UWidgetAnimation* WidgetAnimation = AnimationExport.Get<UWidgetAnimation>();
			if (!WidgetAnimation) continue;
			
			const FString AnimationName = WidgetAnimation->GetName();
			if (AnimationName.EndsWith(TEXT("_INST"))) {
				WidgetAnimation->Rename(*AnimationName.Mid(0, AnimationName.Len() - 5), WidgetBlueprint);
			}

			WidgetBlueprint->Animations.Add(WidgetAnimation);
		}
	}
}
