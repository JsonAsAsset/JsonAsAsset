/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "PropertyUtilities.h"

class JSONASASSET_API USerializerContainer {
public:
    /* Constructors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    explicit USerializerContainer(UPackage* Package);
    USerializerContainer();

    /* Virtual Constructor */
    virtual ~USerializerContainer() {}

    UPackage* Package;

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Serializer and Property Serializer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
public:
    FORCEINLINE UObjectSerializer* GetObjectSerializer() const;
    FORCEINLINE UPropertySerializer* GetPropertySerializer() const;

public:
    UPropertySerializer* PropertySerializer;
    UObjectSerializer* ObjectSerializer;

private:
    void CreateSerializers();
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Serializer and Property Serializer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
};