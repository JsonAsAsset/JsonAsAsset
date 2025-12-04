/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Utilities/Compatibility.h"
#include "Dom/JsonObject.h"
#include "CoreMinimal.h"
#include "Utilities/Serializers/SerializerContainer.h"

/* ReSharper disable once CppUnusedIncludeDirective */
#include "Macros.h"

/* ReSharper disable once CppUnusedIncludeDirective */
#include "TypesHelper.h"

#include "Registry/RegistrationInfo.h"
#include "Styling/SlateIconFinder.h"
#include "Utilities/AssetUtilities.h"

/* Base handler for converting JSON to assets */
class JSONASASSET_API IImporter : public USerializerContainer {
public:
    /* Constructors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    IImporter() {}

    /* Importer Constructor */
    IImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, 
              const TArray<TSharedPtr<FJsonValue>>& JsonObjects = {});

    virtual ~IImporter() override {}

public:
    /* Overriden in child classes, returns false if failed. */
    virtual bool Import() {
        return false;
    }

    virtual UObject* CreateAsset(UObject* CreatedAsset = nullptr);

    template<typename T>
    T* Create() {
        UObject* TargetAsset = CreateAsset(nullptr);

        return Cast<T>(TargetAsset);
    }

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

template <class T>
TObjectPtr<T> IImporter::DownloadWrapper(TObjectPtr<T> InObject, FString Type, const FString Name, const FString Path) {
    const UJsonAsAssetSettings* Settings = GetSettings();

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
            FJRedirects::Reverse(NewPath);
            
            /* Try importing the asset */
            if (FAssetUtilities::ConstructAsset(FSoftObjectPath(Type + "'" + NewPath + "." + Name + "'").ToString(), FSoftObjectPath(Type + "'" + NewPath + "." + Name + "'").ToString(), Type, InObject, bDownloadStatus)) {
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