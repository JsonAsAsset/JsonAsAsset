/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/DataAssetImporter.h"
#include "Engine/DataAsset.h"

bool IDataAssetImporter::Import() {
	UDataAsset* DataAsset = NewObject<UDataAsset>(Package, AssetClass, FName(AssetName), RF_Public | RF_Standalone);
	auto _ = DataAsset->MarkPackageDirty();

	UObjectSerializer* ObjectSerializer = GetObjectSerializer();
	ObjectSerializer->SetExportForDeserialization(JsonObject, DataAsset);
	ObjectSerializer->Parent = DataAsset;

	ObjectSerializer->DeserializeExports(AllJsonObjects);

	ObjectSerializer->DeserializeObjectProperties(AssetData, DataAsset);
	
	return OnAssetCreation(DataAsset);
}
