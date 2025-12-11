/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "ParentDropdownBuilder.h"

/* Lets you know if your settings aren't setup correctly. */
struct IActionRequiredDropdownBuilder final : IParentDropdownBuilder {
	virtual void Build(FMenuBuilder& MenuBuilder) const override;
};