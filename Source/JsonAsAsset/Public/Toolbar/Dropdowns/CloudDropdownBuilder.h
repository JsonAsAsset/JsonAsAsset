/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "ParentDropdownBuilder.h"

struct ICloudDropdownBuilder final : IParentDropdownBuilder {
	virtual void Build(FMenuBuilder& MenuBuilder) const override;
};