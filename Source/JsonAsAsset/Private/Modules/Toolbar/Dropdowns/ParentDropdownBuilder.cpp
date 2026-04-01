/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Modules/Toolbar/Dropdowns/ParentDropdownBuilder.h"

#include "JsonAsAsset.h"
#include "Modules/Metadata.h"
#include "Engine/Compatibility.h"
#include "Engine/EngineUtilities.h"

void IParentDropdownBuilder::Build(FMenuBuilder& MenuBuilder) const {
	MenuBuilder.BeginSection(
		"JsonAsAssetSection", 
		FText::FromString(FJMetadata::Version)
	);
}
