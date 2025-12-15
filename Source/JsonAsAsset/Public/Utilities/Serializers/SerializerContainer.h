/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "PropertyUtilities.h"

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

    virtual FString GetAssetName() const;
    virtual FString GetAssetType() const;
    virtual UClass* GetAssetClass();
    
    virtual TSharedPtr<FJsonObject> GetAssetData() const;
    virtual TSharedPtr<FJsonObject>& GetAssetExport();

    virtual UPackage* GetPackage() const;
    virtual void SetPackage(UPackage* NewPackage);

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
};