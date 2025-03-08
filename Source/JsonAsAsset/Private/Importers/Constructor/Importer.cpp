// Copyright JAA Contributors 2024-2025

#include "Importers/Constructor/Importer.h"

#include "Settings/JsonAsAssetSettings.h"

// Content Browser Modules
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"

// Utilities
#include "Utilities/AssetUtilities.h"

#include "Misc/MessageDialog.h"
#include "UObject/SavePackage.h"

// Slate Icons
#include "Styling/SlateIconFinder.h"

// ----> Importers
// Particle System Importing is not finalized
#ifndef JSONASASSET_PARTICLESYSTEM_ALLOW
#define JSONASASSET_PARTICLESYSTEM_ALLOW 0
#endif

#if JSONASASSET_PARTICLESYSTEM_ALLOW
#include "Importers/Types/ParticleSystemImporter.h"
#endif

#include "Importers/Types/Curves/CurveFloatImporter.h"
#include "Importers/Types/Curves/CurveVectorImporter.h"
#include "Importers/Types/Curves/CurveLinearColorImporter.h"
#include "Importers/Types/Curves/CurveLinearColorAtlasImporter.h"

#include "Importers/Types/Materials/MaterialInstanceConstantImporter.h"
#include "Importers/Types/Materials/MaterialFunctionImporter.h"
#include "Importers/Types/Materials/MaterialImporter.h"

#include "Importers/Types/Animation/BlendspaceImporter.h"
#include "Importers/Types/Animation/AnimationBaseImporter.h"

#include "Importers/Types/Tables/DataTableImporter.h"
#include "Importers/Types/Tables/CurveTableImporter.h"

#include "Importers/Types/Audio/SoundCueImporter.h"
#include "Importers/Types/Animation/SkeletonImporter.h"
#include "Importers/Types/Niagara/NiagaraParameterCollectionImporter.h"
#include "Importers/Types/DataAssetImporter.h"
#include "Importers/Types/Physics/PhysicsAssetImporter.h"
#include "Importers/Types/UserDefinedEnumImporter.h"
// <---- Importers

// Templated Class
#include "Importers/Constructor/TemplatedImporter.h"

// ----------------------- Templated Engine Classes ----------------------------------------------
#include "Materials/MaterialParameterCollection.h"
#include "Engine/SubsurfaceProfile.h"
#include "Curves/CurveLinearColor.h"
#include "Logging/MessageLog.h"
#include "Sound/SoundNode.h"
// -----------------------------------------------------------------------------------------------

#define LOCTEXT_NAMESPACE "IImporter"

// Importer Construction
IImporter::IImporter(const FString& FileName, const FString& FilePath, 
		  const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, 
		  UPackage* OutermostPkg, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects)
	: AllJsonObjects(AllJsonObjects), JsonObject(JsonObject), FileName(FileName),
	  FilePath(FilePath), Package(Package), OutermostPkg(OutermostPkg), ParentObject(nullptr)
{
	PropertySerializer = NewObject<UPropertySerializer>();
	GObjectSerializer = NewObject<UObjectSerializer>();
	GObjectSerializer->SetPropertySerializer(PropertySerializer);
	
	// Make Properties if it doesn't exist
	if (!JsonObject->HasField(TEXT("Properties"))) {
		JsonObject->SetObjectField(TEXT("Properties"), TSharedPtr<FJsonObject>());
	}

	TSharedPtr<FJsonObject> Properties = JsonObject->GetObjectField(TEXT("Properties"));

	// Move asset properties defined outside "Properties" and move it inside
	for (const auto& Pair : JsonObject->Values)
	{
		const FString& PropertyName = Pair.Key;
    
		if (!PropertyName.Equals(TEXT("Type")) &&
			!PropertyName.Equals(TEXT("Name")) &&
			!PropertyName.Equals(TEXT("Class")) &&
			!PropertyName.Equals(TEXT("Flags")) &&
			!PropertyName.Equals(TEXT("Properties")))
		{
			Properties->SetField(PropertyName, Pair.Value);
		}
	}
}

// -----------------------------------------------------------------------------------------------

TArray<FString> ImporterAcceptedTypes = {
	"# Animation Assets", /* Animation Assets ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

	"AnimSequence",
	"AnimMontage",
	"BlendSpace",
	
	"# Curve Assets", /* Curve Assets ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	
	"CurveTable",
	"CurveFloat",
	"CurveVector",
	"CurveLinearColor",
	"CurveLinearColorAtlas",

	"# Data Assets", /* Data Assets ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

	"DataAsset",
	"DataTable",

	"", /* separator */
	
	"LandscapeGrassType",

	"# Material Assets", /* Material Assets ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

	"Material",
	"MaterialFunction",
	"MaterialInstanceConstant",
	"MaterialParameterCollection",

	"", /* separator */
	
	"SubsurfaceProfile",
	"NiagaraParameterCollection",
	
	"# Skeletal Assets", /* Skeletal Assets ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

	"SkeletalMeshLODSettings",
	"Skeleton",

	"", /* separator */
	
	"SlateBrushAsset",
	"SlateWidgetStyleAsset",

#if JSONASASSET_PARTICLESYSTEM_ALLOW
	"# Particle Assets", /* Particle Assets ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

	"ParticleSystem",
#endif

	"# Physics Assets", /* Physics Assets ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	
	"PhysicsAsset",
	"PhysicalMaterial",

	"# Sound Assets", /* Sound Assets ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

	"SoundCue",
	"ReverbEffect",
	"SoundAttenuation",
	"SoundConcurrency",
	"SoundClass",
	"SoundMix",
	"SoundModulationPatch",

	"# Texture Assets", /* Texture Assets ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

	"TextureRenderTarget2D",
	
	"# User Defined Assets", /* User Defined Assets ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

	"UserDefinedEnum"
};

// Handles the JSON of a file.
// I want to replace Handle with Import in most of these functions
bool IImporter::ImportExports(TArray<TSharedPtr<FJsonValue>> Exports, FString File, const bool bHideNotifications) const
{
	TArray<FString> Types;
	for (const TSharedPtr<FJsonValue>& Obj : Exports) Types.Add(Obj->AsObject()->GetStringField(TEXT("Type")));

	for (const TSharedPtr<FJsonValue>& ExportPtr : Exports) {
		TSharedPtr<FJsonObject> DataObject = ExportPtr->AsObject();

		FString Type = DataObject->GetStringField(TEXT("Type"));
		FString Name = DataObject->GetStringField(TEXT("Name"));

		UClass* Class = FindObject<UClass>(ANY_PACKAGE, *Type);

		if (Class == nullptr) continue;

		/* If the class inherits DataAsset, it is importable */
		bool InheritsDataAsset = Class->IsChildOf(UDataAsset::StaticClass());

		if (!CanImport(Type) || InheritsDataAsset) continue;

		// Convert from relative to full
		// NOTE: Used for references
		if (FPaths::IsRelative(File)) File = FPaths::ConvertRelativePathToFull(File);

		UPackage* LocalOutermostPkg;
		UPackage* LocalPackage = FAssetUtilities::CreateAssetPackage(Name, File, LocalOutermostPkg);

		/* Importer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
		IImporter* Importer = nullptr;
		
		/* Try to find the importer using a factory delegate */
		if (const ImporterFactoryDelegate* Factory = FindFactoryForAssetType(Type)) {
			Importer = (*Factory)(Name, File, DataObject, LocalPackage, LocalOutermostPkg, Exports);
		}

		/* If it inherits DataAsset, use the data asset importer */
		if (!Importer && InheritsDataAsset) {
			Importer = new IDataAssetImporter(Class, Name, File, DataObject, LocalPackage, LocalOutermostPkg, Exports);
		}
		
		if (Importer == nullptr) {
			Importer = new ITemplatedImporter<UObject>(Class, Name, File, DataObject, LocalPackage, LocalOutermostPkg, Exports);
		}

		bool Successful = false;

		try {
			Successful = Importer->Import();
		} catch (const char* Exception) {
			UE_LOG(LogJson, Error, TEXT("Importer exception: %s"), *FString(Exception));
		}

		if (Successful) {
			UE_LOG(LogJson, Log, TEXT("Successfully imported \"%s\" as \"%s\""), *Name, *Type);
			
			if (!(Type == "AnimSequence" || Type == "AnimMontage"))
				Importer->SavePackage();

			// Notification for asset
			AppendNotification(
				FText::FromString("Imported type: " + Type),
				FText::FromString(Name),
				2.0f,
				FSlateIconFinder::FindCustomIconBrushForClass(FindObject<UClass>(nullptr, *("/Script/Engine." + Type)), TEXT("ClassThumbnail")),
				SNotificationItem::CS_Success,
				false,
				350.0f
			);

			FMessageLog MessageLogger = FMessageLog(FName("JsonAsAsset"));

			MessageLogger.Message(EMessageSeverity::Info, FText::FromString("Imported Asset: " + Name + " (" + Type + ")"));
		} else AppendNotification(
			FText::FromString("Import Failed: " + Type),
			FText::FromString(Name),
			2.0f,
			FSlateIconFinder::FindCustomIconBrushForClass(FindObject<UClass>(nullptr, *("/Script/Engine." + Type)), TEXT("ClassThumbnail")),
			SNotificationItem::CS_Fail,
			false,
			350.0f
		);
	}

	return true;
}

TArray<TSharedPtr<FJsonValue>> IImporter::GetObjectsWithTypeStartingWith(const FString& StartsWithStr) {
	TArray<TSharedPtr<FJsonValue>> FilteredObjects;

	for (const TSharedPtr<FJsonValue>& JsonObjectValue : AllJsonObjects) {
		if (JsonObjectValue->Type == EJson::Object) {
			TSharedPtr<FJsonObject> JsonObjectType = JsonObjectValue->AsObject();

			if (JsonObjectType.IsValid() && JsonObjectType->HasField(TEXT("Type"))) {
				FString TypeValue = JsonObjectType->GetStringField(TEXT("Type"));

				// Check if the "Type" field starts with the specified string
				if (TypeValue.StartsWith(StartsWithStr)) {
					FilteredObjects.Add(JsonObjectValue);
				}
			}
		}
	}

	return FilteredObjects;
}

// This is called at the end of asset creation, bringing the user to the asset and fully loading it
bool IImporter::HandleAssetCreation(UObject* Asset) const {
	FAssetRegistryModule::AssetCreated(Asset);
	if (!Asset->MarkPackageDirty()) return false;
	
	Package->SetDirtyFlag(true);
	Asset->PostEditChange();
	Asset->AddToRoot();
	
	Package->FullyLoad();

	// Browse to newly added Asset
	const TArray<FAssetData>& Assets = {Asset};
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().SyncBrowserToAssets(Assets);

	return true;
}

// Function to check if an asset needs to be imported. Once imported, the asset will be set and returned.
template <typename T>
TObjectPtr<T> IImporter::DownloadWrapper(TObjectPtr<T> InObject, FString Type, FString Name, FString Path) {
	const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();
	bool bEnableLocalFetch = Settings->bEnableLocalFetch;

	FMessageLog MessageLogger = FMessageLog(FName("JsonAsAsset"));

	if (bEnableLocalFetch && (
		InObject == nullptr ||
			Settings->AssetSettings.TextureImportSettings.bDownloadExistingTextures &&
			Type == "Texture2D"
		)
	) {
		const UObject* DefaultObject = T::StaticClass()->ClassDefaultObject;

		if (DefaultObject != nullptr && !Name.IsEmpty() && !Path.IsEmpty()) {
			bool bRemoteDownloadStatus = false;

			// Notification
			if (FAssetUtilities::ConstructAsset(FSoftObjectPath(Type + "'" + Path + "." + Name + "'").ToString(), Type, InObject, bRemoteDownloadStatus)) {
				const FText AssetNameText = FText::FromString(Name);
				const FSlateBrush* IconBrush = FSlateIconFinder::FindCustomIconBrushForClass(FindObject<UClass>(nullptr, *("/Script/Engine." + Type)), TEXT("ClassThumbnail"));

				if (bRemoteDownloadStatus) {
					AppendNotification(
						FText::FromString("Locally Downloaded: " + Type),
						AssetNameText,
						2.0f,
						IconBrush,
						SNotificationItem::CS_Success,
						false,
						310.0f
					);

					MessageLogger.Message(EMessageSeverity::Info, FText::FromString("Downloaded asset: " + Name + " (" + Type + ")"));
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

					MessageLogger.Error(FText::FromString("Failed to download asset: " + Name + " (" + Type + ")"));
				}
			}
		}
	}

	return InObject;
}

// Loads a single <T> object ptr -------------------------------------------------------
template void IImporter::LoadObject<UMaterialInterface>(const TSharedPtr<FJsonObject>*, TObjectPtr<UMaterialInterface>&);
template void IImporter::LoadObject<USubsurfaceProfile>(const TSharedPtr<FJsonObject>*, TObjectPtr<USubsurfaceProfile>&);
template void IImporter::LoadObject<UTexture>(const TSharedPtr<FJsonObject>*, TObjectPtr<UTexture>&);
template void IImporter::LoadObject<UMaterialParameterCollection>(const TSharedPtr<FJsonObject>*, TObjectPtr<UMaterialParameterCollection>&);
template void IImporter::LoadObject<UAnimSequence>(const TSharedPtr<FJsonObject>*, TObjectPtr<UAnimSequence>&);
template void IImporter::LoadObject<USoundWave>(const TSharedPtr<FJsonObject>*, TObjectPtr<USoundWave>&);
template void IImporter::LoadObject<UObject>(const TSharedPtr<FJsonObject>*, TObjectPtr<UObject>&);
template void IImporter::LoadObject<UMaterialFunctionInterface>(const TSharedPtr<FJsonObject>*, TObjectPtr<UMaterialFunctionInterface>&);
template void IImporter::LoadObject<USoundNode>(const TSharedPtr<FJsonObject>*, TObjectPtr<USoundNode>&);

template <typename T>
void IImporter::LoadObject(const TSharedPtr<FJsonObject>* PackageIndex, TObjectPtr<T>& Object) {
	FString ObjectType, ObjectName, ObjectPath, Outer;
	PackageIndex->Get()->GetStringField(TEXT("ObjectName")).Split("'", &ObjectType, &ObjectName);
	PackageIndex->Get()->GetStringField(TEXT("ObjectPath")).Split(".", &ObjectPath, nullptr);

	const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();

	// Rare case of needing a GameName
	if (!Settings->AssetSettings.GameName.IsEmpty()) {
		ObjectPath = ObjectPath.Replace(*(Settings->AssetSettings.GameName + "/Content"), TEXT("/Game"));
	}

	ObjectPath = ObjectPath.Replace(TEXT("Engine/Content"), TEXT("/Engine"));
	ObjectName = ObjectName.Replace(TEXT("'"), TEXT(""));

	if (ObjectName.Contains(".")) {
		ObjectName.Split(".", nullptr, &ObjectName);
	}

	if (ObjectName.Contains(".")) {
		ObjectName.Split(".", &Outer, &ObjectName);
	}

	// Try to load object using the object path and the object name combined
	TObjectPtr<T> LoadedObject = Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *(ObjectPath + "." + ObjectName)));

	if (!Outer.IsEmpty())
	{
		AActor* NewLoadedObject = Cast<AActor>(ParentObject);
		auto Components = NewLoadedObject->GetComponents();
		
		for (UActorComponent* Component : Components)
		{
			if (ObjectName == Component->GetName())
			{
				LoadedObject = Cast<T>(Component);
			}
		}
	}
	
	// Material Expression case
	if (!LoadedObject && ObjectName.Contains("MaterialExpression")) {
		FString AssetName;
		ObjectPath.Split("/", nullptr, &AssetName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		LoadedObject = Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *(ObjectPath + "." + AssetName + ":" + ObjectName)));
	}

	Object = LoadedObject;

	if (!Object)
	{
		Object = DownloadWrapper(LoadedObject, ObjectType, ObjectName, ObjectPath);
	}
}

// Loads an array of <T> object ptrs -------------------------------------------------------
template TArray<TObjectPtr<UCurveLinearColor>> IImporter::LoadObject<UCurveLinearColor>(const TArray<TSharedPtr<FJsonValue>>&, TArray<TObjectPtr<UCurveLinearColor>>);

template <typename T>
TArray<TObjectPtr<T>> IImporter::LoadObject(const TArray<TSharedPtr<FJsonValue>>& PackageArray, TArray<TObjectPtr<T>> Array) {
	for (const TSharedPtr<FJsonValue>& ArrayElement : PackageArray) {
		const TSharedPtr<FJsonObject> ObjectPtr = ArrayElement->AsObject();

		FString ObjectType, ObjectName, ObjectPath;
		ObjectPtr->GetStringField(TEXT("ObjectName")).Split("'", &ObjectType, &ObjectName);
		ObjectPtr->GetStringField(TEXT("ObjectPath")).Split(".", &ObjectPath, nullptr);
		ObjectName = ObjectName.Replace(TEXT("'"), TEXT(""));

		TObjectPtr<T> LoadedObject = Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *(ObjectPath + "." + ObjectName)));
		Array.Add(DownloadWrapper(LoadedObject, ObjectType, ObjectName, ObjectPath));
	}

	return Array;
}

// Sends off to the ImportExports function once read
void IImporter::ImportReference(const FString& File) const
{
	/* ----  Parse JSON into UE JSON Reader ---- */
	FString ContentBefore;
	FFileHelper::LoadFileToString(ContentBefore, *File);

	FString Content = FString(TEXT("{\"data\": "));
	Content.Append(ContentBefore);
	Content.Append(FString("}"));

	TSharedPtr<FJsonObject> JsonParsed;
	const TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Content);
	/* ---------------------------------------- */

	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed)) {
		const TArray<TSharedPtr<FJsonValue>> DataObjects = JsonParsed->GetArrayField(TEXT("data"));

		ImportExports(DataObjects, File);
	}
}

TMap<FName, FExportData> IImporter::CreateExports() {
	TMap<FName, FExportData> OutExports;

	for (const TSharedPtr<FJsonValue> Value : AllJsonObjects) {
		TSharedPtr<FJsonObject> Object = TSharedPtr<FJsonObject>(Value->AsObject());

		FString ExType = Object->GetStringField(TEXT("Type"));
		FString Name = Object->GetStringField(TEXT("Name"));
		FString Outer = "None";

		if (Object->HasField(TEXT("Outer"))) {
			Outer = Object->GetStringField(TEXT("Outer"));
		}

		OutExports.Add(FName(Name), FExportData(ExType, Outer, Object));
	}

	return OutExports;
}

// Called before HandleAssetCreation, simply saves the asset if user opted
void IImporter::SavePackage() const
{
	const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();

	// Ensure the package is valid before proceeding
	if (Package == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Package is null"));
		return;
	}

	const FString PackageName = Package->GetName();
	const FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

	// User option to save packages on import
	if (Settings->AssetSettings.bSavePackagesOnImport) {
#if ENGINE_MAJOR_VERSION >= 5
		FSavePackageArgs SaveArgs; {
			SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
			SaveArgs.Error = GError;
			SaveArgs.SaveFlags = SAVE_NoError;
		}
		
		UPackage::SavePackage(Package, nullptr, *PackageFileName, SaveArgs);
#else
		UPackage::SavePackage(Package, nullptr, RF_Standalone, *PackageFileName);
#endif
	}
}

bool IImporter::OnAssetCreation(UObject* Asset) const
{
	SavePackage();
	
	return HandleAssetCreation(Asset);
}

FName IImporter::GetExportNameOfSubobject(const FString& PackageIndex) {
	FString Name;
	PackageIndex.Split("'", nullptr, &Name);
	Name.Split(":", nullptr, &Name);
	Name = Name.Replace(TEXT("'"), TEXT(""));
	return FName(Name);
}

TArray<TSharedPtr<FJsonValue>> IImporter::FilterExportsByOuter(const FString& Outer) {
	TArray<TSharedPtr<FJsonValue>> ReturnValue = TArray<TSharedPtr<FJsonValue>>();

	for (const TSharedPtr<FJsonValue> Value : AllJsonObjects) {
		const TSharedPtr<FJsonObject> ValueObject = TSharedPtr<FJsonObject>(Value->AsObject());

		FString ExOuter;
		if (ValueObject->TryGetStringField(TEXT("Outer"), ExOuter) && ExOuter == Outer) 
			ReturnValue.Add(TSharedPtr<FJsonValue>(Value));
	}

	return ReturnValue;
}

TSharedPtr<FJsonValue> IImporter::GetExportByObjectPath(const TSharedPtr<FJsonObject>& Object) {
	const TSharedPtr<FJsonObject> ValueObject = TSharedPtr<FJsonObject>(Object);

	FString StringIndex; {
		ValueObject->GetStringField(TEXT("ObjectPath")).Split(".", nullptr, &StringIndex);
	}

	return AllJsonObjects[FCString::Atod(*StringIndex)];
}

#undef LOCTEXT_NAMESPACE