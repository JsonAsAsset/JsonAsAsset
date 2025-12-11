/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "ParentDropdownBuilder.h"

struct IGeneralDropdownBuilder final : IParentDropdownBuilder {
	virtual void Build(FMenuBuilder& MenuBuilder) const override;
};