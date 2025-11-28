/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "PropertyUtilities.h"

class JSONASASSET_API USerializerContainer {
public:
    /* Constructors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    explicit USerializerContainer(UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects);
    USerializerContainer();

    /* Virtual Constructor */
    virtual ~USerializerContainer() {}
    
    FUObjectExportContainer AssetContainer;
    TArray<TSharedPtr<FJsonValue>> AllJsonObjects;

    /* AssetExport ~~~~~~~~~~~~~~~> */
public:
    FUObjectExport AssetExport;

    virtual FString GetAssetName() const;
    virtual FString GetAssetType() const;
    virtual UClass* GetAssetClass();
    
    virtual TSharedPtr<FJsonObject> GetAssetData() const;
    virtual TSharedPtr<FJsonObject> GetAssetExport() const;

    virtual UObject* GetParent() const;
    virtual void SetParent(UObject* Parent);

    /* Serializer ~~~~~~~~~~~~~~~> */
public:
    FORCEINLINE UObjectSerializer* GetObjectSerializer() const;
    FORCEINLINE UPropertySerializer* GetPropertySerializer() const;
    
protected:
    void CreateSerializer();
    
private:
    UObjectSerializer* ObjectSerializer;

    // testing
public:
    UPackage* GetPackage() const {
        return Package;
    }

    void SetPackage(UPackage* NewPackage) {
        Package = NewPackage;
    }
    
private:
    UPackage* Package;
};