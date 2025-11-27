/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Utilities/Serializers/SerializerContainer.h"

/* Serializer Constructor */
USerializerContainer::USerializerContainer(UPackage* Package, UPackage* OutermostPackage) : Package(Package), OutermostPackage(OutermostPackage) {
	CreateSerializers();
}

USerializerContainer::USerializerContainer() : USerializerContainer(nullptr, nullptr) {
}

UObjectSerializer* USerializerContainer::GetObjectSerializer() const {
	return ObjectSerializer;
}

UPropertySerializer* USerializerContainer::GetPropertySerializer() const {
	return GetObjectSerializer()->PropertySerializer;
}

void USerializerContainer::CreateSerializers() {
	PropertySerializer = NewObject<UPropertySerializer>();
	ObjectSerializer = NewObject<UObjectSerializer>();
	ObjectSerializer->SetPropertySerializer(PropertySerializer);
}
