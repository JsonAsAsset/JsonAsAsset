/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Importers/Types/Texture/TextureImporter.h"
#include "Engine/TextureLightProfile.h"

/* Explicit instantiation of ITemplatedImporter for UObject */
template class ITextureImporter<UTexture>;
template class ITextureImporter<UTextureLightProfile>;

template <typename AssetType>
bool ITextureImporter<AssetType>::Import() {
	TObjectPtr<AssetType> T;
	DownloadWrapperAsync<AssetType>(T, "TextureLightProfile", GetAssetName(), GetPackage()->GetPathName(), [](TObjectPtr<AssetType>){ });
	
	return true;
}
