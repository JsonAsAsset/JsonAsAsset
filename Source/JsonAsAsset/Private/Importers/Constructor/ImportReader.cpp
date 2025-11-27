/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Constructor/ImportReader.h"

#include "Importers/Constructor/Importer.h"
#include "Importers/Constructor/TemplatedImporter.h"
#include "Importers/Types/DataAssetImporter.h"
#include "Importers/Types/Texture/TextureImporter.h"
#include "Styling/SlateIconFinder.h"
#include "Utilities/AssetUtilities.h"
#include "Utilities/EngineUtilities.h"

bool IImportReader::ReadExportsAndImport(TArray<TSharedPtr<FJsonValue>> Exports, const FString& File, bool bHideNotifications) {
	for (const TSharedPtr<FJsonValue>& ExportPtr : Exports) {
		if (ExportPtr->Type != EJson::Object) continue;
		
		ReadExportAndImport(Exports, ExportPtr->AsObject(), File, bHideNotifications);
	}

	return true;
}

void IImportReader::ReadExportAndImport(const TArray<TSharedPtr<FJsonValue>>& Exports,
	const TSharedPtr<FJsonObject>& Export, FString File, const bool bHideNotifications)
{
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
	UPackage* LocalOutermostPackage;
	UPackage* LocalPackage = FAssetUtilities::CreateAssetPackage(Name, File, LocalOutermostPackage, FailureReason);

	if (LocalPackage == nullptr) {
		/* Try fixing our Export Directory Settings using the provided File directory if local package not found */
        UJsonAsAssetSettings* PluginSettings = GetMutableDefault<UJsonAsAssetSettings>();

		PluginSettings->ReadAppData();
		LocalPackage = FAssetUtilities::CreateAssetPackage(Name, File, LocalOutermostPackage, FailureReason);

		if (LocalPackage == nullptr) {
			FString ExportDirectoryCache = PluginSettings->Runtime.ExportDirectory.Path;
		
			if (FString DirectoryPathFix; File.Split(TEXT("Output/Exports/"), &DirectoryPathFix, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd)) {
				DirectoryPathFix = DirectoryPathFix + TEXT("Output/Exports");

				PluginSettings->Runtime.ExportDirectory.Path = DirectoryPathFix;
				SavePluginConfig(PluginSettings);

				/* Retry creating the asset package */
				LocalPackage = FAssetUtilities::CreateAssetPackage(Name, File, LocalOutermostPackage, FailureReason);

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
		Importer = (*Factory)(File, Export, LocalPackage, LocalOutermostPackage, Exports);
	}

	/* If it inherits DataAsset, use the data asset importer */
	if (Importer == nullptr && InheritsDataAsset) {
		Importer = new IDataAssetImporter(File, Export, LocalPackage, LocalOutermostPackage, Exports);
	}

	/* By default, (with no existing importer) use the templated importer with the asset class. */
	if (Importer == nullptr) {
		Importer = new ITemplatedImporter<UObject>(
			File, Export, LocalPackage, LocalOutermostPackage, Exports
		);
	}

	/* TODO: Don't hardcode this. */
	if (IsAssetTypeImportableUsingCloud(Type)) {
		Importer = new ITextureImporter<UTextureLightProfile>(
			File, Export, LocalPackage, LocalOutermostPackage, Exports
		);
	}

	/*if (ObjectSerializer != nullptr && PropertySerializer != nullptr) {
		Importer->PropertySerializer->ExportsContainer = GetPropertySerializer()->ExportsContainer;
		Importer->PropertySerializer->Importer = Importer;
	}

	Package = LocalPackage;*/
	/*SetupImportTracking();*/

	/* Import the asset ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	bool Successful = false; {
		try {
			Successful = Importer->Import();
		} catch (const char* Exception) {
			UE_LOG(LogJsonAsAsset, Error, TEXT("Importer exception: %s"), *FString(Exception));
		}
	}

	/*if (Importer) {
		if (Importer->ImportedAsset != nullptr) {
			ImportedAsset = Importer->ImportedAsset;
		}
	}*/

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

void IImportReader::ImportReference(const FString& File) {
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
