/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Constructor/TextureImporter.h"

/* Explicit instantiation of ITemplatedImporter for UObject */
template class ITemplatedImporter<UObject>;

template <typename AssetType>
bool ITemplatedImporter<AssetType>::Import() {
	AssetType* Asset = NewObject<AssetType>(Package, AssetClass ? AssetClass : AssetType::StaticClass(), FName(AssetName), RF_Public | RF_Standalone);

	Asset->MarkPackageDirty();

	UObjectSerializer* ObjectSerializer = GetObjectSerializer();
	ObjectSerializer->SetExportForDeserialization(JsonObject, Asset);
	ObjectSerializer->Parent = Asset;

	ObjectSerializer->DeserializeExports(AllJsonObjects);
	
	GetObjectSerializer()->DeserializeObjectProperties(AssetData, Asset);

	return OnAssetCreation(Asset);
}
