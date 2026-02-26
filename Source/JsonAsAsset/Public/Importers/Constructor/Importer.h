/* Copyright JsonAsAsset Contributors 2024-2026 */

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
    void LoadExport(const TSharedPtr<FJsonObject>* PackageIndex, TFunction<void(TObjectPtr<T>)> OnComplete);

    /* Loads an array of <T> object ptrs */
    template<class T = UObject>
    TArray<TObjectPtr<T>> LoadExport(const TArray<TSharedPtr<FJsonValue>>& PackageArray, TArray<TObjectPtr<T>> Array);

public:
    void Save() const;

    /*
     * Handle edit changes, and add it to the content browser
     */
    bool OnAssetCreation(UObject* Asset) const;
    
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Serializer and Property Serializer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
public:
    /* Function to check if an asset needs to be imported. Once imported, the asset will be set and returned. */
    template <class T = UObject>
    FORCEINLINE static void DownloadWrapper(TObjectPtr<T> InObject, FString Type, const FString Name, const FString Path, TFunction<void(TObjectPtr<T>)> OnComplete);

protected:
    FORCEINLINE FUObjectExportContainer GetExportContainer() const;
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Object Serializer and Property Serializer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
};

template <class T>
void IImporter::DownloadWrapper(TObjectPtr<T> InObject, FString Type, const FString Name, const FString Path, TFunction<void(TObjectPtr<T>)> OnComplete) {
    const UJsonAsAssetSettings* Settings = GetSettings();

    if (Type == "Texture") Type = "Texture2D";

    if (!Settings->EnableCloudServer || (Path.StartsWith(TEXT("Engine/")) || Path.StartsWith(TEXT("/Engine/")))) {
        OnComplete(InObject);
        return;
    }

    const bool bShouldDownload = InObject == nullptr ||  Settings->AssetSettings.Texture.UpdateExisingTextures && Type == TEXT("Texture2D");

    if (!bShouldDownload) {
        OnComplete(InObject);
        return;
    }
    
    const UObject* DefaultObject = GetClassDefaultObject(T::StaticClass());
    if (!DefaultObject || Name.IsEmpty() || Path.IsEmpty()) {
        OnComplete(InObject);
        return;
    }
    
    FString NewPath = Path;
    FJRedirects::Reverse(NewPath);
    
    const FString FullSoftPath = FSoftObjectPath(Type + TEXT("'") + NewPath + TEXT(".") + Name + TEXT("'")).ToString();
    
    FAssetUtilities::ConstructAssetAsync<T>( FullSoftPath, FullSoftPath, Type, [this, Name, Type, InObject, OnComplete](TObjectPtr<T> DownloadedObject, bool bSuccess) {
        TObjectPtr<T> FinalObject = DownloadedObject ? DownloadedObject : InObject;

        const FText AssetNameText = FText::FromString(Name);

        const FSlateBrush* IconBrush =
            FSlateIconFinder::FindCustomIconBrushForClass(
                FindObject<UClass>(nullptr, *("/Script/Engine." + Type)),
                TEXT("ClassThumbnail"));

        if (bSuccess) {
            AppendNotification(
                FText::FromString(TEXT("Locally Downloaded: ") + Type),
                AssetNameText,
                2.0f,
                IconBrush,
                SNotificationItem::CS_Success,
                false,
                310.0f);
        } else {
            AppendNotification(
                FText::FromString(TEXT("Download Failed: ") + Type),
                AssetNameText,
                5.0f,
                IconBrush,
                SNotificationItem::CS_Fail,
                false,
                310.0f);
        }

        OnComplete(FinalObject);
    });

    return InObject;
}