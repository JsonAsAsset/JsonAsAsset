/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Modules/Toolbar/Dropdowns/CloudToolsDropdownBuilder.h"

#include "Utilities/EngineUtilities.h"

#include "Modules/Cloud/Tools/AnimationData.h"
#include "Modules/Cloud/Tools/ConvexCollision.h"
#include "Modules/Cloud/Tools/SkeletalMeshData.h"

void ICloudToolsDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	static TSkeletalMeshData* SkeletalMeshTool;

	if (SkeletalMeshTool == nullptr) {
		SkeletalMeshTool = new TSkeletalMeshData();
	}
	
	MenuBuilder.BeginSection("JsonAsAssetCloudToolsSection", FText::FromString("Cloud Tools"));
	
	MenuBuilder.AddMenuEntry(
		FText::FromString("Import Static Mesh Properties"),
		FText::FromString("Imports collision, properties and more using Cloud and applies it to the corresponding assets in the content browser folder."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

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
		FText::FromString("Import Animation Data"),
		FText::FromString("Imports Animation Data using Cloud and applies it to the corresponding assets in the content browser folder."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

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
		FText::FromString("Import Skeletal Mesh Data"),
		FText::FromString("Imports Skeletal Mesh Data using Cloud and applies it to the corresponding assets in the content browser folder."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.BspMode"),

		FUIAction(
			FExecuteAction::CreateLambda([] {
				if (SkeletalMeshTool != nullptr)
				{
					SkeletalMeshTool->Execute();
				}
			}),
			FCanExecuteAction::CreateLambda([this] {
				return Cloud::Status::IsOpened();
			})
		),
		NAME_None
	);

	MenuBuilder.EndSection();
}
