/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Utilities/Serializers/PropertyUtilities.h"
#include "Dom/JsonObject.h"

struct FTextureCreatorUtilities {
public:
	FTextureCreatorUtilities(const FString& AssetName, const FString& FilePath, UPackage* Package, const bool bUseOctetStream)
		: bUseOctetStream(bUseOctetStream), AssetName(AssetName), FilePath(FilePath), Package(Package)
	{
		PropertySerializer = NewObject<UPropertySerializer>();
		ObjectSerializer = NewObject<UObjectSerializer>();

		ObjectSerializer->SetPropertySerializer(PropertySerializer);
	}

	bool bUseOctetStream = true;

    template <class T = UObject>
	bool CreateTexture(UTexture*& OutTexture, TArray<uint8>& Data, const TSharedPtr<FJsonObject>& Properties);
	bool CreateTextureCube(UTexture*& OutTextureCube, const TArray<uint8>& Data, const TSharedPtr<FJsonObject>& Properties) const;
	bool CreateVolumeTexture(UTexture*& OutVolumeTexture, TArray<uint8>& Data, const TSharedPtr<FJsonObject>& Properties) const;
	bool CreateRenderTarget2D(UTexture*& OutRenderTarget2D, const TSharedPtr<FJsonObject>& Properties) const;

	/* Deserialization functions */
	bool DeserializeTexture2D(UTexture2D* InTexture2D, const TSharedPtr<FJsonObject>& Properties) const;
	bool DeserializeTexture(UTexture* Texture, const TSharedPtr<FJsonObject>& Properties) const;
	bool DeserializeTexturePlatformData(UTexture* Texture, TArray<uint8>& Data, FTexturePlatformData& TexturePlatformData, const TSharedPtr<FJsonObject>& Properties);

private:
	static void GetDecompressedTextureData(uint8* Data, uint8*& OutData, const int SizeX, const int SizeY, const int SizeZ, const int TotalSize, const EPixelFormat Format);

protected:
	FString AssetName;
	FString FilePath;
	UPackage* Package;
	UPropertySerializer* PropertySerializer;
	UObjectSerializer* ObjectSerializer;

public:
	FORCEINLINE UObjectSerializer* GetObjectSerializer() const { return ObjectSerializer; }
};
