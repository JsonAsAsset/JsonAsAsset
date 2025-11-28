/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/DataAssetImporter.h"
#include "Engine/DataAsset.h"

UObject* IDataAssetImporter::CreateAsset(UObject* CreatedAsset) {
	return IImporter::CreateAsset(NewObject<UDataAsset>(GetPackage(), GetAssetClass(), FName(GetAssetName()), RF_Public | RF_Standalone));
}

bool IDataAssetImporter::Import() {
	UDataAsset* DataAsset = Create<UDataAsset>();
	auto _ = DataAsset->MarkPackageDirty();

	GetObjectSerializer()->SetExportForDeserialization(GetAssetExport(), DataAsset);
	GetObjectSerializer()->Parent = DataAsset;

	GetObjectSerializer()->DeserializeExports(JsonObjects);

	GetObjectSerializer()->DeserializeObjectProperties(GetAssetData(), DataAsset);
	
	return OnAssetCreation(DataAsset);
}