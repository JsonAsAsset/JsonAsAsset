/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/Texture/TextureImporter.h"

/* Explicit instantiation of ITemplatedImporter for UObject */
template class ITextureImporter<UTexture>;

template <typename AssetType>
bool ITextureImporter<AssetType>::Import() {
	TObjectPtr<AssetType> T;
	DownloadWrapper<AssetType>(T, "TextureLightProfile", AssetName, Package->GetPathName());
	
	return true;
}
