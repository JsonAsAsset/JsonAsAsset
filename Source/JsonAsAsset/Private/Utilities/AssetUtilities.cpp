/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Utilities/AssetUtilities.h"

#include "Importers/Constructor/Importer.h"

#include "Utilities/Textures/TextureCreatorUtilities.h"

#include "Curves/CurveLinearColor.h"
#include "Sound/SoundNode.h"
#include "Engine/SubsurfaceProfile.h"
#include "Materials/MaterialParameterCollection.h"
#include "Interfaces/IPluginManager.h"
#include "Settings/JsonAsAssetSettings.h"
#include "Dom/JsonObject.h"

#include "UObject/SavePackage.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Modules/CloudModule.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Utilities/RemoteUtilities.h"

/* CreateAssetPackage Implementations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
UPackage* FAssetUtilities::CreateAssetPackage(const FString& FullPath) {
	UPackage* Package = CreatePackage(
		/* 4.25, 4.26.0 and below need an Outer */
#if UE4_25_BELOW || (UE4_26_0)
		nullptr, 
#endif
		*FullPath);
	Package->FullyLoad();

	return Package;
}

UPackage* FAssetUtilities::CreateAssetPackage(const FString& Name, const FString& OutputPath, UPackage*& OutOutermostPkg, FString& FailureReason) {
	const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();
	
	FString ModifiablePath = OutputPath;
	
	/* References Automatically Formatted */
	if (!ModifiablePath.StartsWith("/Game/") && !ModifiablePath.StartsWith("/Plugins/") && ModifiablePath.Contains("/Content/")) {
		if (!Settings->AssetSettings.GameName.IsEmpty()) {
			ModifiablePath = ModifiablePath.Replace(*(Settings->AssetSettings.GameName + "/Content"), TEXT("/Game"));
			ModifiablePath.Split(*(Settings->ExportDirectory.Path + "/"), nullptr, &ModifiablePath, ESearchCase::IgnoreCase, ESearchDir::FromStart);
			ModifiablePath.Split("/", &ModifiablePath, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			ModifiablePath += "/";
		}

		if (!ModifiablePath.StartsWith("/Game/") && !ModifiablePath.StartsWith("/Plugins/") && ModifiablePath.Contains("/Content/")) {
			ModifiablePath.Split(*(Settings->ExportDirectory.Path + "/"), nullptr, &ModifiablePath, ESearchCase::IgnoreCase, ESearchDir::FromStart);
			ModifiablePath.Split("/", nullptr, &ModifiablePath, ESearchCase::IgnoreCase, ESearchDir::FromStart);
			ModifiablePath.Split("/", &ModifiablePath, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			/* Ex: RestPath: Plugins/Folder/BaseTextures */
			/* Ex: RestPath: Content/SecondaryFolder */
			const bool bIsPlugin = ModifiablePath.StartsWith("Plugins");

			/* Plugins/Folder/BaseTextures -> Folder/BaseTextures */
			if (bIsPlugin) {
				FString PluginName = ModifiablePath;
				FString RemainingPath;
				/* PluginName = TestName */
				/* RemainingPath = SetupAssets/Materials */
				ModifiablePath.Split("/Content/", &PluginName, &RemainingPath, ESearchCase::IgnoreCase, ESearchDir::FromStart);
				PluginName.Split("/", nullptr, &PluginName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

				/* /PluginName/Materials */
				ModifiablePath = PluginName + "/" + RemainingPath;
			}
			/* Content/SecondaryFolder -> Game/SecondaryFolder */
			else {
				ModifiablePath = ModifiablePath.Replace(TEXT("Content"), TEXT("Game"));
			}

			ModifiablePath = "/" + ModifiablePath + "/";

			/* Check if plugin exists */
			if (bIsPlugin && !ModifiablePath.StartsWith("/Game/")) {
				FString PluginName;
				ModifiablePath.Split("/", nullptr, &PluginName, ESearchCase::IgnoreCase, ESearchDir::FromStart);
				PluginName.Split("/", &PluginName, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromStart);

				if (IPluginManager::Get().FindPlugin(PluginName) == nullptr)
					CreatePlugin(PluginName);
			}
		}
	} else {
		FString RootName; {
			ModifiablePath.Split("/", nullptr, &RootName, ESearchCase::IgnoreCase, ESearchDir::FromStart);
			RootName.Split("/", &RootName, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromStart);
		}

		if (RootName != "Game" && RootName != "Engine" && IPluginManager::Get().FindPlugin(RootName) == nullptr) {
			CreatePlugin(RootName);
		}

		ModifiablePath.Split("/", &ModifiablePath, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

		ModifiablePath = ModifiablePath + "/";
	}

	const FString PathWithGame = ModifiablePath + Name;

	if (PathWithGame.Contains(TEXT("//"), ESearchCase::CaseSensitive)) {
		FailureReason = "Attempted to create a package with name containing double slashes.\n\nUpdate your configuration to use a valid Export Directory.";
		return nullptr;
	}
	
	UPackage* Package = CreateAssetPackage(*PathWithGame);
	OutOutermostPkg = Package->GetOutermost();
	Package->FullyLoad();

	return Package;
}

UPackage* FAssetUtilities::CreateAssetPackage(const FString& Name, const FString& OutputPath) {
	UPackage* Ignore = nullptr; /* Put here because &nullptr doesn't work */
	FString StringIgnore = "";
	
	return CreateAssetPackage(Name, OutputPath, Ignore, StringIgnore);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
template bool FAssetUtilities::ConstructAsset<UMaterialInterface>(const FString& Path, const FString& RealPath, const FString& Type, TObjectPtr<UMaterialInterface>& OutObject, bool& bSuccess);
template bool FAssetUtilities::ConstructAsset<USubsurfaceProfile>(const FString& Path, const FString& RealPath, const FString& Type, TObjectPtr<USubsurfaceProfile>& OutObject, bool& bSuccess);
template bool FAssetUtilities::ConstructAsset<UTexture>(const FString& Path, const FString& RealPath, const FString& Type, TObjectPtr<UTexture>& OutObject, bool& bSuccess);
template bool FAssetUtilities::ConstructAsset<UAnimSequence>(const FString& Path, const FString& RealPath, const FString& Type, TObjectPtr<UAnimSequence>& OutObject, bool& bSuccess);
template bool FAssetUtilities::ConstructAsset<UMaterialParameterCollection>(const FString& Path, const FString& RealPath, const FString& Type, TObjectPtr<UMaterialParameterCollection>& OutObject, bool& bSuccess);
template bool FAssetUtilities::ConstructAsset<USoundWave>(const FString& Path, const FString& RealPath, const FString& Type, TObjectPtr<USoundWave>& OutObject, bool& bSuccess);
template bool FAssetUtilities::ConstructAsset<UObject>(const FString& Path, const FString& RealPath, const FString& Type, TObjectPtr<UObject>& OutObject, bool& bSuccess);
template bool FAssetUtilities::ConstructAsset<UMaterialFunctionInterface>(const FString& Path, const FString& RealPath, const FString& Type, TObjectPtr<UMaterialFunctionInterface>& OutObject, bool& bSuccess);
template bool FAssetUtilities::ConstructAsset<USoundNode>(const FString& Path, const FString& RealPath, const FString& Type, TObjectPtr<USoundNode>& OutObject, bool& bSuccess);
template bool FAssetUtilities::ConstructAsset<UCurveLinearColor>(const FString& Path, const FString& RealPath, const FString& Type, TObjectPtr<UCurveLinearColor>& OutObject, bool& bSuccess);
template bool FAssetUtilities::ConstructAsset<UTextureLightProfile>(const FString&, const FString&, const FString&, TObjectPtr<UTextureLightProfile>&, bool&);

/* Importing assets from Cloud */
template <typename T>
bool FAssetUtilities::ConstructAsset(const FString& Path, const FString& RealPath, const FString& Type, TObjectPtr<T>& OutObject, bool& bSuccess) {
	if (Type.IsEmpty()) {
		return false;
	}

	/* Supported Texture Classes */
	const bool bIsTexture = Type ==
		"Texture2D" ||
		Type == "TextureRenderTarget2D" ||
		Type == "TextureCube" ||
		Type == "VolumeTexture" ||
		Type == "TextureLightProfile";

	FString GamePath = Path;
    ReverseRedirectPath(GamePath);

	/* Supported Assets */
	if (IImporter::CanImport(Type, true) || bIsTexture) {
		if (bIsTexture) {
			UTexture* Texture;
			const FString NewPath = RealPath;

			FString RootName; {
				NewPath.Split("/", nullptr, &RootName, ESearchCase::IgnoreCase, ESearchDir::FromStart);
				RootName.Split("/", &RootName, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromStart);
			}

			/* Missing Plugin: Create it */
			if (RootName != "Game" && RootName != "Engine" && IPluginManager::Get().FindPlugin(RootName) == nullptr) {
				CreatePlugin(RootName);
			}

			bSuccess = Construct_TypeTexture(NewPath, Path, Texture);
			if (bSuccess) OutObject = Cast<T>(Texture);

			return true;
		}
		
		const TSharedPtr<FJsonObject> Response = API_RequestExports(Path);
		if (Response == nullptr || Path.IsEmpty()) return true;

		if (Response->HasField(TEXT("errored"))) {
			UE_LOG(LogJsonAsAsset, Log, TEXT("Error from response \"%s\""), *Path);
			return true;
		}

		const TSharedPtr<FJsonObject> JsonObject = Response->GetArrayField(TEXT("exports"))[0]->AsObject();
		FString PackagePath;
		FString AssetName;
		RealPath.Split(".", &PackagePath, &AssetName);

		if (JsonObject) {
			const FString NewPath = PackagePath;

			FString RootName; {
				NewPath.Split("/", nullptr, &RootName, ESearchCase::IgnoreCase, ESearchDir::FromStart);
				RootName.Split("/", &RootName, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromStart);
			}

			if (RootName != "Game" && RootName != "Engine" && IPluginManager::Get().FindPlugin(RootName) == nullptr) {
				CreatePlugin(RootName);
			}

			/* Import asset by IImporter */
			IImporter* Importer = new IImporter();
			bSuccess = Importer->ReadExportsAndImport(Response->GetArrayField(TEXT("exports")), PackagePath, true);

			/* Define found object */
			OutObject = Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *RealPath));

			return OutObject != nullptr;
		}
	}

	return false;
}

bool FAssetUtilities::Construct_TypeTexture(const FString& Path, const FString& FetchPath, UTexture*& OutTexture) {
	if (Path.IsEmpty()) {
		return false;
	}

	const TSharedPtr<FJsonObject> JsonObject = API_RequestExports(FetchPath);
	if (JsonObject == nullptr) {
		return false;
	}

	TArray<TSharedPtr<FJsonValue>> Response = JsonObject->GetArrayField(TEXT("exports"));
	if (Response.Num() == 0) {
		return false;
	}

	const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();
	const TSharedPtr<FJsonObject> JsonExport = Response[0]->AsObject();
	const FString Type = JsonExport->GetStringField(TEXT("Type"));
	
	TArray<uint8> Data = TArray<uint8>();
	bool bUseOctetStream = Type == "TextureLightProfile"
	                       || Type == "TextureCube"
	                       || Type == "VolumeTexture"
	                       || Type == "TextureRenderTarget2D";

	/* ~~~~~~~~~~~~~~~ Download Texture Data ~~~~~~~~~~~~ */
	if (Type != "TextureRenderTarget2D") {
		FHttpModule* HttpModule = &FHttpModule::Get();
		const auto HttpRequest = HttpModule->CreateRequest();

		HttpRequest->SetURL(CloudModule::URL + "/api/export?path=" + FetchPath);
		HttpRequest->SetHeader("content-type", bUseOctetStream ? "application/octet-stream" : "image/png");
		HttpRequest->SetVerb(TEXT("GET"));
		
		const auto HttpResponse = FRemoteUtilities::ExecuteRequestSync(HttpRequest);

		if (!HttpResponse.IsValid() || HttpResponse->GetResponseCode() != 200)
			return false;

		if (HttpResponse->GetContentType().StartsWith("application/json; charset=utf-8")) {
			return false;
		}

		Data = HttpResponse->GetContent();
		if (Data.Num() == 0) {
			return false;
		}
	}

	return Fast_Construct_TypeTexture(JsonExport, Path, Type, Data, OutTexture);
}

bool FAssetUtilities::Fast_Construct_TypeTexture(const TSharedPtr<FJsonObject>& JsonExport, const FString& Path, const FString& Type, TArray<uint8> Data, UTexture*& OutTexture) {
	const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();
	UTexture* Texture = nullptr;
	
	FString PackagePath;
	FString AssetName; {
		Path.Split(".", &PackagePath, &AssetName);
	}

	const bool bUseOctetStream = Type == "TextureLightProfile"
					   || Type == "TextureCube"
					   || Type == "VolumeTexture"
					   || Type == "TextureRenderTarget2D";

	UPackage* Package = CreateAssetPackage(*PackagePath);
	UPackage* OutermostPkg = Package->GetOutermost();
	Package->FullyLoad();

	FTextureCreatorUtilities TextureCreator = FTextureCreatorUtilities(AssetName, Path, Package, OutermostPkg, bUseOctetStream);

	if (Type == "Texture2D") {
		TextureCreator.CreateTexture<UTexture2D>(Texture, Data, JsonExport);
	}
	if (Type == "TextureLightProfile") {
		TextureCreator.CreateTexture<UTextureLightProfile>(Texture, Data, JsonExport);
	}
	if (Type == "TextureCube") {
		TextureCreator.CreateTextureCube(Texture, Data, JsonExport);
	}
	if (Type == "VolumeTexture") {
		TextureCreator.CreateVolumeTexture(Texture, Data, JsonExport);
	}
	if (Type == "TextureRenderTarget2D") {
		TextureCreator.CreateRenderTarget2D(Texture, JsonExport->GetObjectField(TEXT("Properties")));
	}

	if (Texture == nullptr) {
		return false;
	}

	FAssetRegistryModule::AssetCreated(Texture);
	if (!Texture->MarkPackageDirty()) {
		return false;
	}

	Package->SetDirtyFlag(true);
	Texture->PostEditChange();
	Texture->AddToRoot();
	Package->FullyLoad();

	/* Save texture */
	if (Settings->AssetSettings.bSavePackagesOnImport) {
		SavePackage(Package);
	}

	OutTexture = Texture;

	return true;
}

TSharedPtr<FJsonObject> FAssetUtilities::API_RequestExports(const FString& Path, const FString& FetchPath) {
	FHttpModule* HttpModule = &FHttpModule::Get();
	
	const auto HttpRequest = HttpModule->CreateRequest();

	FString PackagePath;
	FString AssetName;
	Path.Split(".", &PackagePath, &AssetName);
	
	const auto NewRequest = HttpModule->CreateRequest();

	NewRequest->SetURL(CloudModule::URL + FetchPath + Path);
	NewRequest->SetVerb(TEXT("GET"));
	
	const auto NewResponse = FRemoteUtilities::ExecuteRequestSync(NewRequest);

	if (!NewResponse.IsValid()) return TSharedPtr<FJsonObject>();

	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(NewResponse->GetContentAsString());
	TSharedPtr<FJsonObject> JsonObject;
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject)) {
		return JsonObject;
	}

	return TSharedPtr<FJsonObject>();
}
