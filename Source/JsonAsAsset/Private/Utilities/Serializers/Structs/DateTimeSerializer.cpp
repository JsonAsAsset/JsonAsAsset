/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Utilities/Serializers/Structs/DateTimeSerializer.h"

void FDateTimeSerializer::Deserialize(UScriptStruct* Struct, void* StructData, const TSharedPtr<FJsonObject> JsonValue) {
	FDateTime* DateTime = static_cast<FDateTime*>(StructData);
	const int64 Ticks = FCString::Atoi64(*JsonValue->GetStringField(TEXT("Ticks")));
	*DateTime = FDateTime(Ticks);
}