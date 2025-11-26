/* Copyright JAA Contributors 2024~2025 */

#pragma once

#include "Utilities/Compatibility.h"
#include "Utilities/EngineUtilities.h"
#include "Dom/JsonObject.h"
#include "CoreMinimal.h"
#include "Styling/SlateIconFinder.h"
#include "Utilities/AssetUtilities.h"
#include "Utilities/Serializers/SerializerContainer.h"

/* TODO: Really feeling like I should rewrite all of this */

/* AssetType/Category ~ Defined in CPP */
extern TMap<FString, TArray<FString>> ImporterTemplatedTypes;

inline TArray<FString> BlacklistedCloudTypes = {
    "AnimSequence",
    "AnimMontage",
    "AnimBlueprintGeneratedClass"
};

inline TArray<FString> ExtraCloudTypes = {
    "TextureLightProfile"
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
    static bool IsAssetTypeImportableUsingCloud(const FString& ImporterType) {
        if (ExtraCloudTypes.Contains(ImporterType)) {
            return true;
        }

        return false;
    }
    
    static bool CanImportWithCloud(const FString& ImporterType) {
        if (BlacklistedCloudTypes.Contains(ImporterType)) {
            return false;
        }

        if (ExtraCloudTypes.Contains(ImporterType)) {
            return true;
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
        }
        
        for (const TPair<FString, TArray<FString>>& Pair : ImporterTemplatedTypes) {
            if (Pair.Value.Contains(ImporterType)) {
                return true;
            }
        }

        if (!Class) {
		    Class = FindClassByType(ImporterType);
        }

        if (Class == nullptr) return false;

        if (ImporterType == "MaterialInterface") return true;

        if (IsAssetTypeImportableUsingCloud(ImporterType)) {
            return true;
        }
        
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
    void ImportReference(const FString& File);

    /*
     * Searches for importable asset types and imports them.
     */
    bool ReadExportsAndImport(TArray<TSharedPtr<FJsonValue>> Exports, const FString& File, bool bHideNotifications = false);
    void ReadExportAndImport(const TArray<TSharedPtr<FJsonValue>>& Exports, const TSharedPtr<FJsonObject>& Export, FString File, bool bHideNotifications = false);

public:
    TArray<TSharedPtr<FJsonValue>> ImportMap;
    
    void SetupImportTracking();

public:
    TArray<TSharedPtr<FJsonValue>> GetObjectsWithPropertyNameStartingWith(const FString& StartsWithStr, const FString& PropertyName);
    TArray<TSharedPtr<FJsonValue>> FilterObjectsWithoutMatchingPropertyName(const FString& StartsWithStr, const FString& PropertyName);

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
    static TObjectPtr<T> DownloadWrapper(TObjectPtr<T> InObject, FString Type, const FString Name, const FString Path) {
        const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();

        /* TODO: Remove this? */
        if (Type == "Texture") Type = "Texture2D";
        
        if (Settings->bEnableCloudServer && (
            InObject == nullptr ||
                Settings->AssetSettings.Texture.bReDownloadTextures &&
                Type == "Texture2D"
            )
            && !Path.StartsWith("Engine/") && !Path.StartsWith("/Engine/")
        ) {
            const UObject* DefaultObject = GetClassDefaultObject(T::StaticClass());

            if (DefaultObject != nullptr && !Name.IsEmpty() && !Path.IsEmpty()) {
                bool bDownloadStatus = false;

                FString NewPath = Path;
                ReverseRedirectPath(NewPath);

                /* Try importing the asset */
                if (FAssetUtilities::ConstructAsset(FSoftObjectPath(Type + "'" + NewPath + "." + Name + "'").ToString(), FSoftObjectPath(Type + "'" + Path + "." + Name + "'").ToString(), Type, InObject, bDownloadStatus)) {
                    const FText AssetNameText = FText::FromString(Name);
                    const FSlateBrush* IconBrush = FSlateIconFinder::FindCustomIconBrushForClass(FindObject<UClass>(nullptr, *("/Script/Engine." + Type)), TEXT("ClassThumbnail"));

                    if (bDownloadStatus) {
                        AppendNotification(
                            FText::FromString("Locally Downloaded: " + Type),
                            AssetNameText,
                            2.0f,
                            IconBrush,
                            SNotificationItem::CS_Success,
                            false,
                            310.0f
                        );

                        GetMessageLog().Message(EMessageSeverity::Info, FText::FromString("Locally Downloaded Asset: " + Name + " (" + Type + ")"));
                    } else {
                        AppendNotification(
                            FText::FromString("Download Failed: " + Type),
                            AssetNameText,
                            5.0f,
                            IconBrush,
                            SNotificationItem::CS_Fail,
                            false,
                            310.0f
                        );

                        GetMessageLog().Error(FText::FromString("Failed to locally download asset: " + Name + " (" + Type + ")"));
                    }
                }
            }
        }

        return InObject;
    }

protected:
    void DeserializeExports(UObject* Parent, bool bCreateObjects = true);
    
    FUObjectExportContainer GetExportContainer() const {
        return GetObjectSerializer()->GetPropertySerializer()->ExportsContainer;
    }
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Serializer and Property Serializer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
};