/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"

/* Default: Does the first section in the dropdown */
struct IParentDropdownBuilder {
	virtual ~IParentDropdownBuilder() = default;
	virtual void Build(FMenuBuilder& MenuBuilder) const;
};