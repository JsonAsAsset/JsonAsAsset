/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Utilities/Serializers/SerializerContainer.h"

/* Serializer Constructor */
USerializerContainer::USerializerContainer(UPackage* Package, UPackage* OutermostPkg)
	: Package(Package), OutermostPkg(OutermostPkg)
{
	PropertySerializer = NewObject<UPropertySerializer>();
	GObjectSerializer = NewObject<UObjectSerializer>();
	GObjectSerializer->SetPropertySerializer(PropertySerializer);
}

UObjectSerializer* USerializerContainer::GetObjectSerializer() const {
	return GObjectSerializer;
}
