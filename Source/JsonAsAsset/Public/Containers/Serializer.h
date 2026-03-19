/* Copyright JsonAsAsset Contributors 2024-2026 */

#pragma once

#include "CoreMinimal.h"

#include "Serializers/PropertySerializer.h"
#include "Serializers/ObjectSerializer.h"

class JSONASASSET_API USerializerContainer {
public:
    /* Virtual Constructor */
    USerializerContainer();
    
    virtual ~USerializerContainer() {}
    
    virtual void Initialize(FUObjectExport& Export, FUObjectExportContainer& Container);
    
    /* AssetExport ~~~~~~~~~~~~~~~> */
public:
    FUObjectExport AssetExport;
    FUObjectExportContainer AssetContainer;

    /* Helper Functions ~~~~~~~~~~~~~~~> */
public:

    virtual FString GetAssetName() const;
    virtual FString GetAssetType() const;
    virtual UClass* GetAssetClass();
    
    virtual TSharedPtr<FJsonObject> GetAssetData() const;
    virtual FUObjectJsonValueExport GetAssetDataAsValue() const;
    virtual FUObjectJsonValueExport GetAssetAsValue() const;
    virtual TSharedPtr<FJsonObject>& GetAssetExport();

    virtual UPackage* GetPackage() const;
    virtual void SetPackage(UPackage* NewPackage);

    virtual UObject* GetParent() const;
    virtual void SetParent(UObject* Parent);

    virtual UObject* GetAsset();
    
    template <class T>
    T* GetTypedAsset() const;

    /* Serializer ~~~~~~~~~~~~~~~> */
public:
    FORCEINLINE UObjectSerializer* GetObjectSerializer() const;
    FORCEINLINE UPropertySerializer* GetPropertySerializer() const;

    void DeserializeExports(UObject* Parent, bool CreateObjects = true);

    virtual void ApplyModifications() { }
protected:
    void CreateSerializer();
    
private:
    UObjectSerializer* ObjectSerializer;
};
