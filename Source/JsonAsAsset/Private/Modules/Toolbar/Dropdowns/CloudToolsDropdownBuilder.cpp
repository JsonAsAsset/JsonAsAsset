/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Modules/Toolbar/Dropdowns/CloudToolsDropdownBuilder.h"

#include "Utilities/EngineUtilities.h"

#include "Modules/Cloud/Tools/AnimationData.h"
#include "Modules/Cloud/Tools/ConvexCollision.h"
#include "Modules/Cloud/Tools/FontData.h"
#include "Modules/Cloud/Tools/SkeletalMeshData.h"

void ICloudToolsDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	static TSkeletalMeshData* SkeletalMeshTool;

	if (SkeletalMeshTool == nullptr) {
		SkeletalMeshTool = new TSkeletalMeshData();
	}
	
	MenuBuilder.BeginSection("JsonAsAssetCloudSection", FText::FromString("Cloud"));
	
	MenuBuilder.AddMenuEntry(
		FText::FromString("Static Meshes"),
		FText::FromString("Imports collision and other properties"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.StaticMeshActor"),

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

	MenuBuilder.AddMenuEntry(
		FText::FromString("Animations"),
		FText::FromString("Imports curve data, notifies and other properties"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.Animation_24x"),

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

	MenuBuilder.AddMenuEntry(
		FText::FromString("Skeletal Meshes"),
		FText::FromString("Imports sockets and other properties"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.SkeletalMeshComponent"),

		FUIAction(
			FExecuteAction::CreateLambda([] {
				if (SkeletalMeshTool != nullptr) {
					SkeletalMeshTool->Execute();
				}
			}),
			FCanExecuteAction::CreateLambda([this] {
				return Cloud::Status::IsOpened();
			})
		),
		NAME_None
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString("Fonts"),
		FText::FromString("Imports font properties (not vectorized data)"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.FontFace"),

		FUIAction(
			FExecuteAction::CreateLambda([] {
				TToolFontData* Tool = new TToolFontData();
				Tool->Execute();
			}),
			FCanExecuteAction::CreateLambda([this] {
				return Cloud::Status::IsOpened();
			})
		),
		NAME_None
	);

	MenuBuilder.EndSection();
}
