/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Utilities/Serializers/SerializerContainer.h"

/* Serializer Constructor */
USerializerContainer::USerializerContainer(UPackage* Package, UPackage* OutermostPkg)
	: Package(Package), OutermostPkg(OutermostPkg)
{
	PropertySerializer = NewObject<UPropertySerializer>();
	ObjectSerializer = NewObject<UObjectSerializer>();
	ObjectSerializer->SetPropertySerializer(PropertySerializer);
}

UObjectSerializer* USerializerContainer::GetObjectSerializer() const {
	return ObjectSerializer;
}

UPropertySerializer* USerializerContainer::GetPropertySerializer() const {
	return GetObjectSerializer()->PropertySerializer;
}
