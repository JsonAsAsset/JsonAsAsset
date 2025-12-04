/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Constructor/ImportReader.h"

#include "Importers/Constructor/Importer.h"
#include "Importers/Constructor/TemplatedImporter.h"
#include "Importers/Types/DataAssetImporter.h"
#include "Importers/Types/Texture/TextureImporter.h"
#include "Settings/Runtime.h"
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

void IImportReader::ReadExportAndImport(const TArray<TSharedPtr<FJsonValue>>& Exports, const TSharedPtr<FJsonObject>& Export, FString File, const bool bHideNotifications) {
	const FString Type = Export->GetStringField(TEXT("Type"));
	FString Name = Export->GetStringField(TEXT("Name"));

	/* BlueprintGeneratedClass is post-fixed with _C */
	if (Type.Contains("BlueprintGeneratedClass")) {
		Name.Split("_C", &Name, nullptr, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
	}

	const UClass* Class = FindClassByType(Type);
	
	if (Class == nullptr) return;

	/* Check if this export can be imported */
	const bool InheritsDataAsset = Class->IsChildOf(UDataAsset::StaticClass());
	if (!CanImport(Type, false, Class)) return;

	/* Convert from relative path to full path */
	if (FPaths::IsRelative(File)) File = FPaths::ConvertRelativePathToFull(File);

	FString FailureReason;
	UPackage* LocalPackage = FAssetUtilities::CreateAssetPackage(Name, File, FailureReason);

	if (LocalPackage == nullptr) {
		/* Try fixing our Export Directory Settings using the provided File directory if local package not found */
        UJsonAsAssetSettings* PluginSettings = GetSettings();

		GJsonAsAssetRuntime.Update();
		LocalPackage = FAssetUtilities::CreateAssetPackage(Name, File, FailureReason);

		if (LocalPackage == nullptr) {
			FString ExportDirectoryCache = GJsonAsAssetRuntime.ExportDirectory.Path;
		
			if (FString DirectoryPathFix; File.Split(TEXT("Output/Exports/"), &DirectoryPathFix, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd)) {
				DirectoryPathFix = DirectoryPathFix + TEXT("Output/Exports");

				GJsonAsAssetRuntime.ExportDirectory.Path = DirectoryPathFix;
				SavePluginConfig(PluginSettings);

				/* Retry creating the asset package */
				LocalPackage = FAssetUtilities::CreateAssetPackage(Name, File, FailureReason);

				/* Undo the change if unsuccessful */
				if (LocalPackage == nullptr) {
					GJsonAsAssetRuntime.ExportDirectory.Path = ExportDirectoryCache;

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
		Importer = (*Factory)(Export, LocalPackage, Exports);
	}

	/* If it inherits DataAsset, use the data asset importer */
	if (Importer == nullptr && InheritsDataAsset) {
		Importer = new IDataAssetImporter(Export, LocalPackage, Exports);
	}

	/* By default, (with no existing importer) use the templated importer with the asset class. */
	if (Importer == nullptr) {
		Importer = new ITemplatedImporter<UObject>(
			Export, LocalPackage, Exports
		);
	}

	/* TODO: Don't hardcode this. */
	if (ImportTypes::Cloud::Extra.Contains(Type)) {
		Importer = new ITextureImporter<UTextureLightProfile>(
			Export, LocalPackage, Exports
		);
	}

	/* Import the asset ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	bool Successful = false; {
		try {
			Successful = Importer->Import();
		} catch (const char* Exception) {
			UE_LOG(LogJsonAsAsset, Error, TEXT("Importer exception: %s"), *FString(Exception));
		}
	}

	if (bHideNotifications) {
		return;
	}

	if (Successful) {
		UE_LOG(LogJsonAsAsset, Log, TEXT("Successfully imported \"%s\" as \"%s\""), *Name, *Type);

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
	TArray<TSharedPtr<FJsonValue>> DataObjects; {
		DeserializeJSON(File, DataObjects);
	}
	
	ReadExportsAndImport(DataObjects, File);
}