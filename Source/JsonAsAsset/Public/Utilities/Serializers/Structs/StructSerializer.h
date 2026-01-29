/* Copyright JsonAsAsset Contributors 2024-2026 */

#pragma once

#include "UObject/Object.h"
#include "Json.h"

/** Handles struct serialization */
class JSONASASSET_API FStructSerializer {
public:
	virtual ~FStructSerializer() = default;
	virtual void Deserialize(UScriptStruct* Struct, void* StructData, const TSharedPtr<FJsonObject> JsonValue) = 0;
};