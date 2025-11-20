/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "PropertyUtilities.h"

class JSONASASSET_API USerializerContainer {
public:
    /* Constructors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    USerializerContainer()
        : Package(nullptr), OutermostPkg(nullptr), PropertySerializer(nullptr), ObjectSerializer(nullptr) {}

    /* Importer Constructor */
    explicit USerializerContainer(UPackage* Package, UPackage* OutermostPkg);

    virtual ~USerializerContainer() {}

    UPackage* Package;
    UPackage* OutermostPkg;

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Serializer and Property Serializer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
public:
    FORCEINLINE UObjectSerializer* GetObjectSerializer() const;
    FORCEINLINE UPropertySerializer* GetPropertySerializer() const;

public:
    UPropertySerializer* PropertySerializer;
    UObjectSerializer* ObjectSerializer;
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Serializer and Property Serializer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
};