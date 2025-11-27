/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Utilities/Serializers/SerializerContainer.h"

/* Serializer Constructor */
USerializerContainer::USerializerContainer(UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects) : Package(Package), AllJsonObjects(AllJsonObjects) {
	CreateSerializers();
}

USerializerContainer::USerializerContainer() : USerializerContainer(nullptr, {}) {
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
