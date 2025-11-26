/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "StructSerializer.h"

class UPropertySerializer;

/** Fallback struct serializer using default UE reflection */
class FFallbackStructSerializer : public FStructSerializer {
	UPropertySerializer* PropertySerializer;
public:
	explicit FFallbackStructSerializer(UPropertySerializer* PropertySerializer) : PropertySerializer(PropertySerializer) {};
	virtual void Deserialize(UScriptStruct* Struct, void* StructValue, const TSharedPtr<FJsonObject> JsonValue) override;
};