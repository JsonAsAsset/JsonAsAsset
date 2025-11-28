/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Constructor/Importer.h"

#include "Settings/JsonAsAssetSettings.h"

/* Utilities */
#include "Utilities/AssetUtilities.h"

#include "Misc/MessageDialog.h"

/* Slate Icons */
#include "Styling/SlateIconFinder.h"

/* ~~~~~~~~~~~~~ Templated Engine Classes ~~~~~~~~~~~~~ */
#include "Materials/MaterialParameterCollection.h"
#include "Engine/SubsurfaceProfile.h"
#include "Curves/CurveLinearColor.h"
#include "Logging/MessageLog.h"
#include "Modules/Log.h"
#include "Sound/SoundNode.h"
#include "Utilities/EngineUtilities.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Importer Constructor */
IImporter::IImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, 
		  const TArray<TSharedPtr<FJsonValue>>& JsonObjects)
	: USerializerContainer(Package, JsonObjects)
{
	/* Create Properties field if it doesn't exist */
	if (!JsonObject->HasField(TEXT("Properties"))) {
		JsonObject->SetObjectField(TEXT("Properties"), TSharedPtr<FJsonObject>());
	}

	AssetExport.JsonObject = JsonObject;

	/* Move asset properties defined outside "Properties" and move it inside */
	for (const auto& Pair : JsonObject->Values) {
		const FString& PropertyName = Pair.Key;
    
		if (!PropertyName.Equals(TEXT("Type")) &&
			!PropertyName.Equals(TEXT("Name")) &&
			!PropertyName.Equals(TEXT("Class")) &&
			!PropertyName.Equals(TEXT("Flags")) &&
			!PropertyName.Equals(TEXT("Properties"))
		) {
			AssetExport.GetProperties()->SetField(PropertyName, Pair.Value);
		}
	}

	AssetExport.NameOverride = AssetExport.GetName();
	
	/* BlueprintGeneratedClass is post-fixed with _C */
	if (AssetExport.GetType().ToString().Contains("BlueprintGeneratedClass")) {
		FString NewName; {
			AssetExport.NameOverride.ToString().Split("_C", &NewName, nullptr, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
			AssetExport.NameOverride = FName(*NewName);
		}
	}
}

template void IImporter::LoadExport<UMaterialInterface>(const TSharedPtr<FJsonObject>*, TObjectPtr<UMaterialInterface>&);
template void IImporter::LoadExport<USubsurfaceProfile>(const TSharedPtr<FJsonObject>*, TObjectPtr<USubsurfaceProfile>&);
template void IImporter::LoadExport<UTexture>(const TSharedPtr<FJsonObject>*, TObjectPtr<UTexture>&);
template void IImporter::LoadExport<UMaterialParameterCollection>(const TSharedPtr<FJsonObject>*, TObjectPtr<UMaterialParameterCollection>&);
template void IImporter::LoadExport<UAnimSequence>(const TSharedPtr<FJsonObject>*, TObjectPtr<UAnimSequence>&);
template void IImporter::LoadExport<USoundWave>(const TSharedPtr<FJsonObject>*, TObjectPtr<USoundWave>&);
template void IImporter::LoadExport<UObject>(const TSharedPtr<FJsonObject>*, TObjectPtr<UObject>&);
template void IImporter::LoadExport<UMaterialFunctionInterface>(const TSharedPtr<FJsonObject>*, TObjectPtr<UMaterialFunctionInterface>&);
template void IImporter::LoadExport<USoundNode>(const TSharedPtr<FJsonObject>*, TObjectPtr<USoundNode>&);

UObject* IImporter::CreateAsset(UObject* CreatedAsset) {
	if (CreatedAsset) {
		AssetExport.Object = CreatedAsset;
        
		return CreatedAsset;
	}
    
	return nullptr;
}

template <typename T>
T* IImporter::Create() {
	UObject* TargetAsset = CreateAsset(nullptr);

	return Cast<T>(TargetAsset);
}

template <typename T>
void IImporter::LoadExport(const TSharedPtr<FJsonObject>* PackageIndex, TObjectPtr<T>& Object) {
	FString ObjectType, ObjectName, ObjectPath, Outer;
	PackageIndex->Get()->GetStringField(TEXT("ObjectName")).Split("'", &ObjectType, &ObjectName);

	ObjectPath = PackageIndex->Get()->GetStringField(TEXT("ObjectPath"));
	ObjectPath.Split(".", &ObjectPath, nullptr);

	RedirectPath(ObjectPath);

	const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();

	if (!Settings->AssetSettings.GameName.IsEmpty()) {
		ObjectPath = ObjectPath.Replace(*(Settings->AssetSettings.GameName + "/Content"), TEXT("/Game"));
		ObjectPath = ObjectPath.Replace(*(Settings->AssetSettings.GameName + "/Plugins"), TEXT(""));
		ObjectPath = ObjectPath.Replace(TEXT("/Content"), TEXT(""));
	}

	ObjectPath = ObjectPath.Replace(TEXT("Engine/Content"), TEXT("/Engine"));
	ObjectName = ObjectName.Replace(TEXT("'"), TEXT(""));

	if (ObjectName.Contains(".")) {
		ObjectName.Split(".", nullptr, &ObjectName);
	}

	if (ObjectName.Contains(".")) {
		ObjectName.Split(".", &Outer, &ObjectName);
	}

	/* Try to load object using the object path and the object name combined */
	TObjectPtr<T> LoadedObject = Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *(ObjectPath + "." + ObjectName)));

	if (!LoadedObject) {
		FString NewObjectPath;
		FString ObjectFileName; {
			ObjectPath.Split("/", &NewObjectPath, &ObjectFileName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		}

		NewObjectPath = NewObjectPath + "/" + ObjectName;

		if (ObjectFileName != ObjectName) {
			LoadedObject = Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *(NewObjectPath + "." + ObjectName)));
		}
	}

	if (GetParent() != nullptr) {
		if (!Outer.IsEmpty() && GetParent()->IsA(AActor::StaticClass())) {
			const AActor* NewLoadedObject = Cast<AActor>(GetParent());
			auto Components = NewLoadedObject->GetComponents();
		
			for (UActorComponent* Component : Components) {
				if (ObjectName == Component->GetName()) {
					LoadedObject = Cast<T>(Component);
				}
			}
		}
	}
	
	/* Material Expression case */
	if (!LoadedObject && ObjectName.Contains("MaterialExpression")) {
		FString SplitObjectName;
		ObjectPath.Split("/", nullptr, &SplitObjectName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		LoadedObject = Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *(ObjectPath + "." + SplitObjectName + ":" + ObjectName)));
	}

	Object = LoadedObject;

	if (!Object && GetObjectSerializer() != nullptr && GetPropertySerializer() != nullptr) {
		const FUObjectExport Export = GetPropertySerializer()->ExportsContainer.Find(ObjectName);
		
		if (Export.IsValid() && Export.Object != nullptr && Export.Object->IsA(T::StaticClass())) {
			Object = TObjectPtr<T>(Cast<T>(Export.Object));
		}
	}

	/* If object is still null, send off to Cloud to download */
	if (!Object) {
		Object = DownloadWrapper(LoadedObject, ObjectType, ObjectName, ObjectPath);
	}
}

template TArray<TObjectPtr<UCurveLinearColor>> IImporter::LoadExport<UCurveLinearColor>(const TArray<TSharedPtr<FJsonValue>>&, TArray<TObjectPtr<UCurveLinearColor>>);

template <typename T>
TArray<TObjectPtr<T>> IImporter::LoadExport(const TArray<TSharedPtr<FJsonValue>>& PackageArray, TArray<TObjectPtr<T>> Array) {
	for (const TSharedPtr<FJsonValue>& ArrayElement : PackageArray) {
		const TSharedPtr<FJsonObject> ObjectPtr = ArrayElement->AsObject();

		FString ObjectType, ObjectName, ObjectPath;
		
		ObjectPtr->GetStringField(TEXT("ObjectName")).Split("'", &ObjectType, &ObjectName);
		ObjectPtr->GetStringField(TEXT("ObjectPath")).Split(".", &ObjectPath, nullptr);
		RedirectPath(ObjectPath);

		ObjectName = ObjectName.Replace(TEXT("'"), TEXT(""));

		TObjectPtr<T> LoadedObject = Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *(ObjectPath + "." + ObjectName)));
		Array.Add(DownloadWrapper(LoadedObject, ObjectType, ObjectName, ObjectPath));
	}

	return Array;
}

template <class T>
TObjectPtr<T> IImporter::DownloadWrapper(TObjectPtr<T> InObject, FString Type, const FString Name, const FString Path) {
    const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();

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

void IImporter::Save() const {
	const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();

	/* Ensure the package is valid before proceeding */
	if (GetPackage() == nullptr) {
		UE_LOG(LogJsonAsAsset, Error, TEXT("IImporter::Save: Package is null"));
		return;
	}

	/* User option to save packages on import */
	if (Settings->AssetSettings.bSaveAssets) {
		SavePackage(GetPackage());
	}
}

bool IImporter::OnAssetCreation(UObject* Asset) const {
	const bool Synced = HandleAssetCreation(Asset, GetPackage());
	if (Synced) {
		Save();
	}
	
	return Synced;
}

void IImporter::DeserializeExports(UObject* Parent, const bool bCreateObjects) {
	GetObjectSerializer()->SetExportForDeserialization(GetAssetExport(), Parent);
	GetObjectSerializer()->Parent = Parent;
    
	GetObjectSerializer()->DeserializeExports(JsonObjects, bCreateObjects);
	ApplyModifications();
}

FUObjectExportContainer IImporter::GetExportContainer() const {
	return GetObjectSerializer()->GetPropertySerializer()->ExportsContainer;
}