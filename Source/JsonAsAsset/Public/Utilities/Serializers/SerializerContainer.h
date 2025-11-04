/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "PropertyUtilities.h"

class JSONASASSET_API USerializerContainer {
public:
    /* Constructors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    USerializerContainer()
        : Package(nullptr), OutermostPkg(nullptr), PropertySerializer(nullptr), GObjectSerializer(nullptr) {}

    /* Importer Constructor */
    explicit USerializerContainer(UPackage* Package, UPackage* OutermostPkg);

    virtual ~USerializerContainer() {}

    UPackage* Package;
    UPackage* OutermostPkg;

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Serializer and Property Serializer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
public:
    FORCEINLINE UObjectSerializer* GetObjectSerializer() const;

protected:
    UPropertySerializer* PropertySerializer;
    UObjectSerializer* GObjectSerializer;
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Serializer and Property Serializer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
};