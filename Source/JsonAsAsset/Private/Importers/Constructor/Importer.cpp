/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Constructor/Importer.h"

#include "Settings/JsonAsAssetSettings.h"

/* Content Browser Modules */
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

/* Utilities */
#include "Utilities/AssetUtilities.h"

#include "Misc/MessageDialog.h"
#include "UObject/SavePackage.h"

/* Slate Icons */
#include "Styling/SlateIconFinder.h"

#include "Importers/Types/DataAssetImporter.h"

/* Templated Class */
#include "Importers/Constructor/TextureImporter.h"

/* ~~~~~~~~~~~~~ Templated Engine Classes ~~~~~~~~~~~~~ */
#include "Materials/MaterialParameterCollection.h"
#include "Engine/SubsurfaceProfile.h"
#include "Curves/CurveLinearColor.h"
#include "Importers/Types/Texture/TextureImporter.h"
#include "Logging/MessageLog.h"
#include "Modules/Log.h"
#include "Sound/SoundNode.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#define LOCTEXT_NAMESPACE "IImporter"

/* Importer Constructor */
IImporter::IImporter(const FString& AssetName, const FString& FilePath, 
		  const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, 
		  UPackage* OutermostPkg, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects,
		  UClass* AssetClass)
	: USerializerContainer(Package, OutermostPkg), AllJsonObjects(AllJsonObjects), JsonObject(JsonObject),
	  FilePath(FilePath), AssetClass(AssetClass), AssetName(AssetName),
	  ParentObject(nullptr)
{
	/* Create Properties field if it doesn't exist */
	if (!JsonObject->HasField(TEXT("Properties"))) {
		JsonObject->SetObjectField(TEXT("Properties"), TSharedPtr<FJsonObject>());
	}

	AssetData = JsonObject->GetObjectField(TEXT("Properties"));

	/* Move asset properties defined outside "Properties" and move it inside */
	for (const auto& Pair : JsonObject->Values) {
		const FString& PropertyName = Pair.Key;
    
		if (!PropertyName.Equals(TEXT("Type")) &&
			!PropertyName.Equals(TEXT("Name")) &&
			!PropertyName.Equals(TEXT("Class")) &&
			!PropertyName.Equals(TEXT("Flags")) &&
			!PropertyName.Equals(TEXT("Properties"))
		) {
			AssetData->SetField(PropertyName, Pair.Value);
		}
	}
}

/*
 * Define supported asset class names here
 *
 * An empty string "" is a separator line
 * A string starting with "# ..." is a category
 */
TMap<FString, TArray<FString>> ImporterTemplatedTypes = {
	{
		TEXT("Curve Assets"),
		{
			TEXT("CurveFloat"),
			TEXT("CurveLinearColor"),
		}
	},
	{
		TEXT("Data Assets"),
		{
			TEXT("SlateBrushAsset"),
			TEXT("SlateWidgetStyleAsset"),
			TEXT("AnimBoneCompressionSettings"),
			TEXT("AnimCurveCompressionSettings"),
			TEXT("LandscapeLayerInfoObject"),
			TEXT("HLODProxy")
		}
	},
	{
		TEXT("Landscape Assets"),
		{
			TEXT("LandscapeGrassType"),
			TEXT("FoliageType_InstancedStaticMesh"),
			TEXT("FoliageType_Actor"),
		}
	},
	{
		TEXT("Material Assets"),
		{
			TEXT("MaterialParameterCollection"),
			TEXT("SubsurfaceProfile"),
		}
	},
	{
		TEXT("Skeletal Assets"),
		{
			TEXT("SkeletalMeshLODSettings"),
		}
	},
	{
		TEXT("Physics Assets"),
		{
			TEXT("PhysicalMaterial"),
		}
	},
	{
		TEXT("Sound Assets"),
		{
			TEXT("ReverbEffect"),
			TEXT("SoundAttenuation"),
			TEXT("SoundConcurrency"),
			TEXT("SoundClass"),
			TEXT("SoundMix"),
			TEXT("SoundModulationPatch"),
			TEXT("SubmixEffectDynamicsProcessorPreset"),
		}
	},
	{
		TEXT("Texture Assets"),
		{
			TEXT("TextureRenderTarget2D"),
			TEXT("RuntimeVirtualTexture"),
		}
	},
	{
		TEXT("Sequencer Assets"),
		{
			TEXT("CameraAnim"),
		}
	}
};

bool IImporter::ReadExportsAndImport(TArray<TSharedPtr<FJsonValue>> Exports, const FString& File, const bool bHideNotifications) {
	for (const TSharedPtr<FJsonValue>& ExportPtr : Exports) {
		if (ExportPtr->Type != EJson::Object) continue;
		
		ReadExportAndImport(Exports, ExportPtr->AsObject(), File, bHideNotifications);
	}

	return true;
}

void IImporter::ReadExportAndImport(const TArray<TSharedPtr<FJsonValue>>& Exports, const TSharedPtr<FJsonObject>& Export, FString File, const bool bHideNotifications) {
	const FString Type = Export->GetStringField(TEXT("Type"));
	FString Name = Export->GetStringField(TEXT("Name"));

	/* BlueprintGeneratedClass is post-fixed with _C */
	if (Type.Contains("BlueprintGeneratedClass")) {
		Name.Split("_C", &Name, nullptr, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
	}

	UClass* Class = FindClassByType(Type);
	
	if (Class == nullptr) return;

	/* Check if this export can be imported */
	const bool InheritsDataAsset = Class->IsChildOf(UDataAsset::StaticClass());
	if (!CanImport(Type, false, Class)) return;

	/* Convert from relative path to full path */
	if (FPaths::IsRelative(File)) File = FPaths::ConvertRelativePathToFull(File);

	RedirectPath(File);

	FString FailureReason;
	UPackage* LocalOutermostPkg;
	UPackage* LocalPackage = FAssetUtilities::CreateAssetPackage(Name, File, LocalOutermostPkg, FailureReason);

	if (LocalPackage == nullptr) {
		/* Try fixing our Export Directory Settings using the provided File directory if local package not found */
        UJsonAsAssetSettings* PluginSettings = GetMutableDefault<UJsonAsAssetSettings>();

		PluginSettings->ReadAppData();
		LocalPackage = FAssetUtilities::CreateAssetPackage(Name, File, LocalOutermostPkg, FailureReason);

		if (LocalPackage == nullptr) {
			FString ExportDirectoryCache = PluginSettings->Runtime.ExportDirectory.Path;
		
			if (FString DirectoryPathFix; File.Split(TEXT("Output/Exports/"), &DirectoryPathFix, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd)) {
				DirectoryPathFix = DirectoryPathFix + TEXT("Output/Exports");

				PluginSettings->Runtime.ExportDirectory.Path = DirectoryPathFix;
				SavePluginConfig(PluginSettings);

				/* Retry creating the asset package */
				LocalPackage = FAssetUtilities::CreateAssetPackage(Name, File, LocalOutermostPkg, FailureReason);

				/* Undo the change if unsuccessful */
				if (LocalPackage == nullptr) {
					PluginSettings->Runtime.ExportDirectory.Path = ExportDirectoryCache;

					SavePluginConfig(PluginSettings);
				}
			}
		}
	}

	if (LocalPackage == nullptr) {
		AppendNotification(
			FText::FromString("Import Failed: " + Type),
			FText::FromString(FailureReason),
			4.0f,
			FSlateIconFinder::FindCustomIconBrushForClass(FindObject<UClass>(nullptr, *("/Script/Engine." + Type)), TEXT("ClassThumbnail")),
			SNotificationItem::CS_Fail,
			false,
			350.0f
		);

		return;
	}

	/* Importer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	IImporter* Importer = nullptr;
	
	/* Try to find the importer using a factory delegate */
	if (const FImporterFactoryDelegate* Factory = FindFactoryForAssetType(Type)) {
		Importer = (*Factory)(Name, File, Export, LocalPackage, LocalOutermostPkg, Exports, Class);
	}

	/* If it inherits DataAsset, use the data asset importer */
	if (Importer == nullptr && InheritsDataAsset) {
		Importer = new IDataAssetImporter(Name, File, Export, LocalPackage, LocalOutermostPkg, Exports, Class);
	}

	/* By default, (with no existing importer) use the templated importer with the asset class. */
	if (Importer == nullptr) {
		Importer = new ITemplatedImporter<UObject>(
			Name, File, Export, LocalPackage, LocalOutermostPkg, Exports, Class
		);
	}

	/* TODO: Don't hardcode this. */
	if (IsAssetTypeImportableUsingCloud(Type)) {
		Importer = new ITextureImporter<UTextureLightProfile>(
			Name, File, Export, LocalPackage, LocalOutermostPkg, Exports, Class
		);
	}

	if (ObjectSerializer != nullptr && PropertySerializer != nullptr) {
		Importer->PropertySerializer->ExportsContainer = GetPropertySerializer()->ExportsContainer;
		Importer->PropertySerializer->Importer = Importer;
	}

	Package = LocalPackage;
	SetupImportTracking();

	/* Import the asset ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	bool Successful = false; {
		try {
			Successful = Importer->Import();
		} catch (const char* Exception) {
			UE_LOG(LogJsonAsAsset, Error, TEXT("Importer exception: %s"), *FString(Exception));
		}
	}

	if (Importer) {
		if (Importer->ImportedAsset != nullptr) {
			ImportedAsset = Importer->ImportedAsset;
		}
	}

	if (bHideNotifications) {
		return;
	}

	if (Successful) {
		UE_LOG(LogJsonAsAsset, Log, TEXT("Successfully imported \"%s\" as \"%s\""), *Name, *Type);

		/* TODO: Remove this? */
		if (Type != "AnimSequence" && Type != "AnimMontage") {
			Importer->Save();
		}

		/* Import Successful Notification */
		AppendNotification(
			FText::FromString("Imported: " + Name),
			FText::FromString(Type),
			2.0f,
			FSlateIconFinder::FindCustomIconBrushForClass(FindObject<UClass>(nullptr, *("/Script/Engine." + Type)), TEXT("ClassThumbnail")),
			SNotificationItem::CS_Success,
			false,
			350.0f
		);

		GetMessageLog().Message(EMessageSeverity::Info, FText::FromString("Imported Asset: " + Name + " (" + Type + ")"));
	} else {
		/* Import Failed Notification */
		AppendNotification(
			FText::FromString("Import Failed: " + Name),
			FText::FromString(Type),
			2.0f,
			FSlateIconFinder::FindCustomIconBrushForClass(FindObject<UClass>(nullptr, *("/Script/Engine." + Type)), TEXT("ClassThumbnail")),
			SNotificationItem::CS_Fail,
			false,
			350.0f
		);
	}
}

void IImporter::SetupImportTracking() {
	const UJsonAsAssetSettings* Settings = GetSettings();

	if (Settings->bEnableCloudServer) {
		const TSharedPtr<FJsonObject> Metadata = Cloud::Export::Get(Package->GetPathName(), false, {
			{
				"metadata",
				"true"
			}
		});

		if (Metadata.IsValid()) {
			ImportMap = Metadata->GetArrayField("ImportMap");
		}

		FScopedSlowTask SlowTask(0.1f, FText::FromString("Downloading " + Package->GetName()));
		SlowTask.MakeDialog();
	}
}

TArray<TSharedPtr<FJsonValue>> IImporter::GetObjectsWithPropertyNameStartingWith(const FString& StartsWithStr, const FString& PropertyName) {
	TArray<TSharedPtr<FJsonValue>> FilteredObjects;

	for (const TSharedPtr<FJsonValue>& JsonObjectValue : AllJsonObjects) {
		if (JsonObjectValue->Type == EJson::Object) {
			TSharedPtr<FJsonObject> JsonObjectType = JsonObjectValue->AsObject();

			if (JsonObjectType.IsValid() && JsonObjectType->HasField(PropertyName)) {
				const FString TypeValue = JsonObjectType->GetStringField(PropertyName);

				/* Check if the "Type" field starts with the specified string */
				if (TypeValue.StartsWith(StartsWithStr)) {
					FilteredObjects.Add(JsonObjectValue);
				}
			}
		}
	}

	return FilteredObjects;
}

TArray<TSharedPtr<FJsonValue>> IImporter::FilterObjectsWithoutMatchingPropertyName(const FString& StartsWithStr, const FString& PropertyName) {
	TArray<TSharedPtr<FJsonValue>> FilteredObjects;

	for (const TSharedPtr<FJsonValue>& JsonObjectValue : AllJsonObjects) {
		if (JsonObjectValue->Type == EJson::Object) {
			TSharedPtr<FJsonObject> JsonObjectType = JsonObjectValue->AsObject();

			if (JsonObjectType.IsValid() && JsonObjectType->HasField(PropertyName)) {
				const FString TypeValue = JsonObjectType->GetStringField(PropertyName);

				/* Check if the "Type" field starts with the specified string */
				if (!TypeValue.StartsWith(StartsWithStr)) {
					FilteredObjects.Add(JsonObjectValue);
				}
			}
		}
	}

	return FilteredObjects;
}

bool IImporter::HandleAssetCreation(UObject* Asset) const {
	{
		/* User Failsafe.... */
		const UPackage* OutermostPackage = Asset->GetOutermost();
		const FString PackageName = OutermostPackage->GetName();

		const FString Path = FPackageName::GetLongPackagePath(PackageName);
		if (!Path.StartsWith(TEXT("/")) || Path.Len() < 2)
		{
			SpawnPrompt("User Failsafe (#1)", "To keep crashes from happening due to user setup issues, which would be assumed as the plugin's mishaps.\n\nHere's some reasons why this failsafe happened:\n\n- You didn't export it from FModel\n- Imported it from a random path, not in Exports/.../\n\nPlease reimport next time at the proper location.");
			
			return false;
		}
	}

	FAssetRegistryModule::AssetCreated(Asset);
	
	if (!Asset->MarkPackageDirty()) return false;
	
	Package->SetDirtyFlag(true);
	Asset->PostEditChange();
	Asset->AddToRoot();
	
	Package->FullyLoad();

	/* Browse to newly added Asset in the Content Browser */
	const TArray<FAssetData>& Assets = { Asset };
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().SyncBrowserToAssets(Assets);

	Asset->PostLoad();
	
	return true;
}

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

	if (ParentObject != nullptr) {
		if (!Outer.IsEmpty() && ParentObject->IsA(AActor::StaticClass())) {
			const AActor* NewLoadedObject = Cast<AActor>(ParentObject);
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

	if (!Object && ObjectSerializer != nullptr && PropertySerializer != nullptr) {
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

template TArray<TObjectPtr<UCurveLinearColor>> IImporter::LoadObject<UCurveLinearColor>(const TArray<TSharedPtr<FJsonValue>>&, TArray<TObjectPtr<UCurveLinearColor>>);

template <typename T>
TArray<TObjectPtr<T>> IImporter::LoadObject(const TArray<TSharedPtr<FJsonValue>>& PackageArray, TArray<TObjectPtr<T>> Array) {
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

void IImporter::ImportReference(const FString& File) {
	/* ~~~~  Parse JSON into UE JSON Reader ~~~~ */
	FString ContentBefore;
	FFileHelper::LoadFileToString(ContentBefore, *File);

	FString Content = FString(TEXT("{\"data\": "));
	Content.Append(ContentBefore);
	Content.Append(FString("}"));

	TSharedPtr<FJsonObject> JsonParsed;
	const TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Content);
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed)) {
		const TArray<TSharedPtr<FJsonValue>> DataObjects = JsonParsed->GetArrayField(TEXT("data"));

		ReadExportsAndImport(DataObjects, File);
	}
}

void IImporter::Save() const {
	const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();

	/* Ensure the package is valid before proceeding */
	if (Package == nullptr) {
		UE_LOG(LogJsonAsAsset, Error, TEXT("Package is null"));
		return;
	}

	/* User option to save packages on import */
	if (Settings->AssetSettings.bSaveAssets) {
		SavePackage(Package);
	}
}

bool IImporter::OnAssetCreation(UObject* Asset) {
	ImportedAsset = Asset;
	
	const bool Synced = HandleAssetCreation(Asset);
	if (Synced) {
		Save();
	}
	
	return Synced;
}

void IImporter::DeserializeExports(UObject* Parent, const bool bCreateObjects) {
	GetObjectSerializer()->SetExportForDeserialization(JsonObject, Parent);
	GetObjectSerializer()->Parent = Parent;
    
	GetObjectSerializer()->DeserializeExports(AllJsonObjects, bCreateObjects);
	ApplyModifications();
}

#undef LOCTEXT_NAMESPACE
