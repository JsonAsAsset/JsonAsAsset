/* Copyright JAA Contributors 2024~2025 */

#pragma once

#include "Utilities/Compatibility.h"
#include "Utilities/EngineUtilities.h"
#include "Utilities/JsonUtilities.h"
#include "Dom/JsonObject.h"
#include "CoreMinimal.h"
#include "Utilities/Serializers/SerializerContainer.h"

/* AssetType/Category ~ Defined in CPP */
extern TMap<FString, TArray<FString>> ImporterTemplatedTypes;

inline TArray<FString> BlacklistedCloudTypes = {
    "AnimSequence",
    "AnimMontage",
    // "AnimBlueprintGeneratedClass"
};

inline const TArray<FString> ExperimentalAssetTypes = {
    "AnimBlueprintGeneratedClass"
};

FORCEINLINE uint32 GetTypeHash(const TArray<FString>& Array) {
    uint32 Hash = 0;
    
    for (const FString& Str : Array) {
        Hash = HashCombine(Hash, GetTypeHash(Str));
    }
    
    return Hash;
}

#define REGISTER_IMPORTER(ImporterClass, AcceptedTypes, Category) \
namespace { \
    struct FAutoRegister_##ImporterClass { \
        FAutoRegister_##ImporterClass() { \
            IImporter::FImporterRegistrationInfo Info( FString(Category), &IImporter::CreateImporter<ImporterClass> ); \
            IImporter::GetFactoryRegistry().Add(AcceptedTypes, Info); \
        } \
    }; \
    static FAutoRegister_##ImporterClass AutoRegister_##ImporterClass; \
}

/* Global handler for converting JSON to assets */
class JSONASASSET_API IImporter : public USerializerContainer {
public:
    /* Constructors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    IImporter() : AssetClass(nullptr), ParentObject(nullptr) {}

    /* Importer Constructor */
    IImporter(const FString& AssetName, const FString& FilePath, 
              const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, 
              UPackage* OutermostPkg, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects = {}, UClass* AssetClass = nullptr);

    virtual ~IImporter() override {}

    /* Easy way to find importers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    using FImporterFactoryDelegate = TFunction<IImporter*(const FString& AssetName, const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, UPackage* OutermostPkg, const TArray<TSharedPtr<FJsonValue>>& Exports, UClass* AssetClass)>;

    template <typename T>
    static IImporter* CreateImporter(const FString& AssetName, const FString& FilePath, const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, UPackage* OutermostPkg, const TArray<TSharedPtr<FJsonValue>>& Exports, UClass* AssetClass) {
        return new T(AssetName, FilePath, JsonObject, Package, OutermostPkg, Exports, AssetClass);
    }

    /* Registration info for an importer */
    struct FImporterRegistrationInfo {
        FString Category;
        FImporterFactoryDelegate Factory;

        FImporterRegistrationInfo(const FString& InCategory, const FImporterFactoryDelegate& InFactory)
            : Category(InCategory)
            , Factory(InFactory)
        {
        }

        FImporterRegistrationInfo() = default;
    };

    static TMap<TArray<FString>, FImporterRegistrationInfo>& GetFactoryRegistry() {
        static TMap<TArray<FString>, FImporterRegistrationInfo> Registry;
        
        return Registry;
    }

    static FImporterFactoryDelegate* FindFactoryForAssetType(const FString& AssetType) {
        const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();

        for (auto& Pair : GetFactoryRegistry()) {
            if (!Settings->bEnableExperiments) {
                if (ExperimentalAssetTypes.Contains(AssetType)) return nullptr;
            }
            
            if (Pair.Key.Contains(AssetType)) {
                return &Pair.Value.Factory;
            }
        }
        
        return nullptr;
    }

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
    /*
    * Overriden in child classes.
    * Returns false if failed.
    */
    virtual bool Import() {
        return false;
    }

public:
    /* Accepted Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    static bool CanImportWithCloud(const FString& ImporterType) {
        if (BlacklistedCloudTypes.Contains(ImporterType)) {
            return false;
        }

        return true;
    }
    
    static bool IsAssetTypeExperimental(const FString& ImporterType) {
        if (ExperimentalAssetTypes.Contains(ImporterType)) {
            return false;
        }

        return true;
    }
    
    static bool CanImport(const FString& ImporterType, const bool IsCloud = false, const UClass* Class = nullptr) {
        /* Blacklists for Cloud importing */
        if (IsCloud) {
            if (!CanImportWithCloud(ImporterType)) {
                return false;
            }
        }
        
        if (FindFactoryForAssetType(ImporterType)) {
            return true;
        };
        
        for (const TPair<FString, TArray<FString>>& Pair : ImporterTemplatedTypes) {
            if (Pair.Value.Contains(ImporterType)) {
                return true;
            }
        }

        if (!Class) {
            Class = FindObject<UClass>(ANY_PACKAGE, *ImporterType);
        }

        if (Class == nullptr) return false;

        if (ImporterType == "MaterialInterface") return true;
        
        return Class->IsChildOf(UDataAsset::StaticClass());
    }

    static bool CanImportAny(TArray<FString>& Types) {
        for (FString& Type : Types) {
            if (CanImport(Type)) return true;
        }
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
    static void ImportReference(const FString& File);

    /*
     * Searches for importable asset types and imports them.
     */
    static bool ReadExportsAndImport(TArray<TSharedPtr<FJsonValue>> Exports, FString File, bool bHideNotifications = false);

public:
    TArray<TSharedPtr<FJsonValue>> GetObjectsWithPropertyNameStartingWith(const FString& StartsWithStr, const FString& PropertyName);
    TArray<TSharedPtr<FJsonValue>> FilterObjectsWithoutMatchingPropertyName(const FString& StartsWithStr, const FString& PropertyName);

    UObject* ParentObject;
    
protected:
    /* This is called at the end of asset creation, bringing the user to the asset and fully loading it */
    bool HandleAssetCreation(UObject* Asset) const;
    void SavePackage() const;

    TMap<FName, FExportData> CreateExports();

    /*
     * Handle edit changes, and add it to the content browser
     */
    bool OnAssetCreation(UObject* Asset) const;

    static FName GetExportNameOfSubobject(const FString& PackageIndex);
    TArray<TSharedPtr<FJsonValue>> FilterExportsByOuter(const FString& Outer);
    TSharedPtr<FJsonValue> GetExportByObjectPath(const TSharedPtr<FJsonObject>& Object);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Serializer and Property Serializer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
public:
    /* Function to check if an asset needs to be imported. Once imported, the asset will be set and returned. */
    template <class T = UObject>
    static TObjectPtr<T> DownloadWrapper(TObjectPtr<T> InObject, FString Type, FString Name, FString Path);

protected:
    void DeserializeExports(UObject* ParentAsset) const {
        UObjectSerializer* ObjectSerializer = GetObjectSerializer();
        ObjectSerializer->SetExportForDeserialization(JsonObject, ParentAsset);
        ObjectSerializer->ParentAsset = ParentAsset;
        
        ObjectSerializer->DeserializeExports(AllJsonObjects);
    };
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Serializer and Property Serializer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
};