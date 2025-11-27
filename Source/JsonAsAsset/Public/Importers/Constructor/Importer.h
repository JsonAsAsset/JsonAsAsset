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

/* Global handler for converting JSON to assets */
class JSONASASSET_API IImporter : public USerializerContainer {
public:
    /* Constructors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    IImporter() : AssetClass(nullptr), ParentObject(nullptr) {}

    /* Importer Constructor */
    IImporter(const FString& AssetName, const FString& FilePath, 
              const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, 
              UPackage* OutermostPackage, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects = {}, UClass* AssetClass = nullptr);

    virtual ~IImporter() override {}

public:
    TArray<TSharedPtr<FJsonValue>> AllJsonObjects;

protected:
    /* Class variables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    TSharedPtr<FJsonObject> JsonObject;
    FString FilePath;

    TSharedPtr<FJsonObject> AssetData;
    UClass* AssetClass;
    FString AssetName;
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    
public:
    /* Overriden in child classes, returns false if failed. */
    virtual bool Import() {
        return false;
    }

public:
    /* Loads a single <T> object ptr */
    template<class T = UObject>
    void LoadObject(const TSharedPtr<FJsonObject>* PackageIndex, TObjectPtr<T>& Object);

    /* Loads an array of <T> object ptrs */
    template<class T = UObject>
    TArray<TObjectPtr<T>> LoadObject(const TArray<TSharedPtr<FJsonValue>>& PackageArray, TArray<TObjectPtr<T>> Array);

public:
    /* Sends off to the ReadExportsAndImport function once read */
    void ImportReference(const FString& File);

    /* Searches for importable asset types and imports them. */
    bool ReadExportsAndImport(TArray<TSharedPtr<FJsonValue>> Exports, const FString& File, bool bHideNotifications = false);
    void ReadExportAndImport(const TArray<TSharedPtr<FJsonValue>>& Exports, const TSharedPtr<FJsonObject>& Export, FString File, bool bHideNotifications = false);

public:
    TArray<TSharedPtr<FJsonValue>> ImportMap;
    
    void SetupImportTracking();

public:
    UObject* ParentObject;
    UObject* ImportedAsset;
    
protected:
    /* This is called at the end of asset creation, bringing the user to the asset and fully loading it */
    bool HandleAssetCreation(UObject* Asset) const;
    void Save() const;

    /*
     * Handle edit changes, and add it to the content browser
     */
    bool OnAssetCreation(UObject* Asset);
    
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