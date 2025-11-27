/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/DataAssetImporter.h"
#include "Engine/DataAsset.h"

UObject* IDataAssetImporter::CreateAsset(UObject* CreatedAsset) {
	return IImporter::CreateAsset(NewObject<UDataAsset>(Package, AssetClass, FName(AssetName), RF_Public | RF_Standalone));
}

bool IDataAssetImporter::Import() {
	UDataAsset* DataAsset = Create<UDataAsset>();
	auto _ = DataAsset->MarkPackageDirty();

	GetObjectSerializer()->SetExportForDeserialization(JsonObject, DataAsset);
	GetObjectSerializer()->Parent = DataAsset;

	GetObjectSerializer()->DeserializeExports(AllJsonObjects);

	GetObjectSerializer()->DeserializeObjectProperties(AssetData, DataAsset);
	
	return OnAssetCreation(DataAsset);
}