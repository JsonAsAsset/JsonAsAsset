/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Utilities/Compatibility.h"
#include "Dom/JsonObject.h"
#include "CoreMinimal.h"
#include "Utilities/Serializers/SerializerContainer.h"

/* ReSharper disable once CppUnusedIncludeDirective */
#include "Macros.h"
#include "Registry/RegistrationInfo.h"

/* ReSharper disable once CppUnusedIncludeDirective */
#include "TypesHelper.h"

/* Base handler for converting JSON to assets */
class JSONASASSET_API IImporter : public USerializerContainer {
public:
    /* Constructors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    IImporter() {}

    /* Importer Constructor */
    IImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, 
              const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects = {});

    virtual ~IImporter() override {}

public:
    FString GetAssetName() const;
    FString GetAssetType() const;
    TSharedPtr<FJsonObject> GetAssetData() const;
    TSharedPtr<FJsonObject> GetAssetExport() const;
    UClass* GetAssetClass();

    void SetParent(UObject* Parent);
    UObject* GetParent() const;

public:
    /* Overriden in child classes, returns false if failed. */
    virtual bool Import() {
        return false;
    }

    virtual UObject* CreateAsset(UObject* CreatedAsset = nullptr);

    template<typename T>
    T* Create();

public:
    /* Loads a single <T> object ptr */
    template<class T = UObject>
    void LoadExport(const TSharedPtr<FJsonObject>* PackageIndex, TObjectPtr<T>& Object);

    /* Loads an array of <T> object ptrs */
    template<class T = UObject>
    TArray<TObjectPtr<T>> LoadExport(const TArray<TSharedPtr<FJsonValue>>& PackageArray, TArray<TObjectPtr<T>> Array);

public:
    void Save() const;

    /*
     * Handle edit changes, and add it to the content browser
     */
    bool OnAssetCreation(UObject* Asset) const;
    
    virtual void ApplyModifications() {};

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Serializer and Property Serializer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
public:
    /* Function to check if an asset needs to be imported. Once imported, the asset will be set and returned. */
    template <class T = UObject>
    FORCEINLINE static TObjectPtr<T> DownloadWrapper(TObjectPtr<T> InObject, FString Type, const FString Name, const FString Path);

protected:
    void DeserializeExports(UObject* Parent, bool bCreateObjects = true);
    
    FORCEINLINE FUObjectExportContainer GetExportContainer() const;
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Serializer and Property Serializer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
};