/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Utilities/Serializers/Structs/FallbackStructSerializer.h"
#include "Utilities/Serializers/PropertyUtilities.h"

void FFallbackStructSerializer::Deserialize(UScriptStruct* Struct, void* StructValue, const TSharedPtr<FJsonObject> JsonValue) {
	for (FProperty* Property = Struct->PropertyLink; Property; Property = Property->PropertyLinkNext) {
		const FString PropertyName = Property->GetName();

		if (PropertySerializer->ShouldDeserializeProperty(Property)) {
			void* PropertyValue = Property->ContainerPtrToValuePtr<void>(StructValue);

			const bool bHasHandledProperty = PassthroughPropertyHandler(Property, PropertyName, PropertyValue, JsonValue, PropertySerializer);

			if (!bHasHandledProperty && JsonValue->HasField(PropertyName)) {
				const TSharedPtr<FJsonValue> ValueObject = JsonValue->Values.FindChecked(PropertyName);

				if (Property->ArrayDim == 1 || ValueObject->Type == EJson::Array) {
					PropertySerializer->DeserializePropertyValue(Property, ValueObject.ToSharedRef(), PropertyValue);
				}
			}
		}
	}
}