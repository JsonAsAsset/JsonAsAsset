/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Constructor/TemplatedImporter.h"

/* Explicit instantiation of ITemplatedImporter for UObject */
template class ITemplatedImporter<UObject>;

template <typename AssetType>
bool ITemplatedImporter<AssetType>::Import() {
	AssetType* Asset = NewObject<AssetType>(Package, AssetClass ? AssetClass : AssetType::StaticClass(), FName(AssetName), RF_Public | RF_Standalone);

	Asset->MarkPackageDirty();

	GetObjectSerializer()->SetExportForDeserialization(JsonObject, Asset);
	GetObjectSerializer()->Parent = Asset;

	GetObjectSerializer()->DeserializeExports(AllJsonObjects);
	
	GetObjectSerializer()->DeserializeObjectProperties(AssetData, Asset);

	return OnAssetCreation(Asset);
}
