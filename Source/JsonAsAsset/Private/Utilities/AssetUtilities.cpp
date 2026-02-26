/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Utilities/AssetUtilities.h"

#include "Importers/Constructor/Importer.h"

#include "Utilities/Textures/TextureCreatorUtilities.h"

#include "Curves/CurveLinearColor.h"
#include "Sound/SoundNode.h"
#include "Engine/SubsurfaceProfile.h"
#include "Materials/MaterialParameterCollection.h"
#include "Settings/JsonAsAssetSettings.h"
#include "Dom/JsonObject.h"

#include "HttpModule.h"
#include "Importers/Constructor/ImportReader.h"
#include "Interfaces/IHttpResponse.h"
#include "Settings/Runtime.h"
#include "Utilities/RemoteUtilities.h"

/* CreateAssetPackage Implementations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
UPackage* FAssetUtilities::CreateAssetPackage(const FString& Path) {
	UPackage* Package = CreatePackage(
		/* 4.25, 4.26.0 and below need an Outer */
#if UE4_25_BELOW || (UE4_26_0)
		nullptr, 
#endif
		*Path);
	Package->FullyLoad();

	return Package;
}

UPackage* FAssetUtilities::CreateAssetPackage(const FString& Name, const FString& OutputPath, FString& FailureReason) {
	const UJsonAsAssetSettings* Settings = GetSettings();
	
	FString ModifiablePath = OutputPath;
	
	/* References Automatically Formatted */
	if (!ModifiablePath.StartsWith("/Game/") && !ModifiablePath.StartsWith("/Plugins/") && ModifiablePath.Contains("/Content/")) {
		if (!Settings->AssetSettings.ProjectName.IsEmpty()) {
			ModifiablePath = ModifiablePath.Replace(*(Settings->AssetSettings.ProjectName + "/Content"), TEXT("/Game"));
			ModifiablePath.Split(*(GJsonAsAssetRuntime.ExportDirectory.Path + "/"), nullptr, &ModifiablePath, ESearchCase::IgnoreCase, ESearchDir::FromStart);
			ModifiablePath.Split("/", &ModifiablePath, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			ModifiablePath += "/";
		}

		if (!ModifiablePath.StartsWith("/Game/") && !ModifiablePath.StartsWith("/Plugins/") && ModifiablePath.Contains("/Content/")) {
			ModifiablePath.Split(*(GJsonAsAssetRuntime.ExportDirectory.Path + "/"), nullptr, &ModifiablePath, ESearchCase::IgnoreCase, ESearchDir::FromStart);
			ModifiablePath.Split("/", nullptr, &ModifiablePath, ESearchCase::IgnoreCase, ESearchDir::FromStart);
			ModifiablePath.Split("/", &ModifiablePath, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			/* Ex: RestPath: Plugins/Folder/BaseTextures */
			/* Ex: RestPath: Content/SecondaryFolder */
			const bool IsPlugin = ModifiablePath.StartsWith("Plugins");

			/* Plugins/Folder/BaseTextures -> Folder/BaseTextures */
			if (IsPlugin) {
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

			FJRedirects::Redirect(ModifiablePath);

			/* Check if plugin exists */
			if (IsPlugin && !ModifiablePath.StartsWith("/Game/")) {
				FString PluginName;
				ModifiablePath.Split("/", nullptr, &PluginName, ESearchCase::IgnoreCase, ESearchDir::FromStart);
				PluginName.Split("/", &PluginName, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromStart);

				if (GetPlugin(PluginName) == nullptr) {
					CreatePlugin(PluginName);
				}
			}
		}
		else {
			FJRedirects::Redirect(ModifiablePath);

			if (!ModifiablePath.StartsWith("/Game/") && !ModifiablePath.StartsWith("/Engine/")) {
				FString PluginName;
				ModifiablePath.Split("/", nullptr, &PluginName, ESearchCase::IgnoreCase, ESearchDir::FromStart);
				PluginName.Split("/", &PluginName, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromStart);

				if (GetPlugin(PluginName) == nullptr) {
					CreatePlugin(PluginName);
				}
			}
		}
	} else {
		FString RootName; {
			ModifiablePath.Split("/", nullptr, &RootName, ESearchCase::IgnoreCase, ESearchDir::FromStart);
			RootName.Split("/", &RootName, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromStart);
		}

		if (RootName != "Game" && RootName != "Engine" && GetPlugin(RootName) == nullptr) {
			CreatePlugin(RootName);
		}

		ModifiablePath.Split("/", &ModifiablePath, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

		ModifiablePath = ModifiablePath + "/";

		FJRedirects::Redirect(ModifiablePath);
	}

	const FString PathWithGame = ModifiablePath + Name;

	if (PathWithGame.Contains(TEXT("//"), ESearchCase::CaseSensitive) || PathWithGame == "None" || PathWithGame.IsEmpty()) {
		FailureReason = "Attempted to create a package with name containing double slashes.\n\nUpdate your configuration to use a valid Export Directory.";
		return nullptr;
	}
	
	UPackage* Package = CreateAssetPackage(*PathWithGame);
	Package->FullyLoad();

	return Package;
}

UPackage* FAssetUtilities::CreateAssetPackage(const FString& Name, const FString& OutputPath) {
	FString StringIgnore = "";
	
	return CreateAssetPackage(Name, OutputPath, StringIgnore);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
template void FAssetUtilities::ConstructAssetAsync<UMaterialInterface>(const FString& Path, const FString& RealPath, const FString& Type, TFunction<void(TObjectPtr<UMaterialInterface>, bool)> OnComplete);
template void FAssetUtilities::ConstructAssetAsync<USubsurfaceProfile>(const FString& Path, const FString& RealPath, const FString& Type, TFunction<void(TObjectPtr<USubsurfaceProfile>, bool)> OnComplete);
template void FAssetUtilities::ConstructAssetAsync<UTexture>(const FString& Path, const FString& RealPath, const FString& Type, TFunction<void(TObjectPtr<UTexture>, bool)> OnComplete);
template void FAssetUtilities::ConstructAssetAsync<UAnimSequence>(const FString& Path, const FString& RealPath, const FString& Type, TFunction<void(TObjectPtr<UAnimSequence>, bool)> OnComplete);
template void FAssetUtilities::ConstructAssetAsync<UMaterialParameterCollection>(const FString& Path, const FString& RealPath, const FString& Type, TFunction<void(TObjectPtr<UMaterialParameterCollection>, bool)> OnComplete);
template void FAssetUtilities::ConstructAssetAsync<USoundWave>(const FString& Path, const FString& RealPath, const FString& Type, TFunction<void(TObjectPtr<USoundWave>, bool)> OnComplete);
template void FAssetUtilities::ConstructAssetAsync<UObject>(const FString& Path, const FString& RealPath, const FString& Type, TFunction<void(TObjectPtr<UObject>, bool)> OnComplete);
template void FAssetUtilities::ConstructAssetAsync<UMaterialFunctionInterface>(const FString& Path, const FString& RealPath, const FString& Type, TFunction<void(TObjectPtr<UMaterialFunctionInterface>, bool)> OnComplete);
template void FAssetUtilities::ConstructAssetAsync<USoundNode>(const FString& Path, const FString& RealPath, const FString& Type, TFunction<void(TObjectPtr<USoundNode>, bool)> OnComplete);
template void FAssetUtilities::ConstructAssetAsync<UCurveLinearColor>(const FString& Path, const FString& RealPath, const FString& Type, TFunction<void(TObjectPtr<UCurveLinearColor>, bool)> OnComplete);
template void FAssetUtilities::ConstructAssetAsync<UTextureLightProfile>(const FString&, const FString&, const FString&, TFunction<void(TObjectPtr<UTextureLightProfile>, bool)> OnComplete);

/* Importing assets from Cloud */
template <typename T>
void FAssetUtilities::ConstructAssetAsync(
	const FString& Path,
	const FString& RealPath,
	const FString& Type,
	TFunction<void(TObjectPtr<T>, bool)> OnComplete)
{
	if (Type.IsEmpty()) {
		OnComplete(nullptr, false);

		return;
	}

	/* Supported Texture Classes */
	const bool IsTexture = Type ==
		"Texture2D" ||
		Type == "TextureRenderTarget2D" ||
		Type == "TextureCube" ||
		Type == "VolumeTexture" ||
		Type == "TextureLightProfile";
	
	if (!CanImport(Type, true) && !IsTexture) {
		OnComplete(nullptr, false);
		return;
	}

	if (IsTexture) {
		const FString NewPath = RealPath;

		FString RootName; {
			NewPath.Split("/", nullptr, &RootName, ESearchCase::IgnoreCase, ESearchDir::FromStart);
			RootName.Split("/", &RootName, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromStart);
		}

		/* Missing Plugin: Create it */
		if (RootName != "Game" && RootName != "Engine" && GetPlugin(RootName) == nullptr) {
			CreatePlugin(RootName);
		}

		CreateTexture(
			NewPath,
			Path,
			[OnComplete](UTexture* Texture, bool bSuccess)
			{
				TObjectPtr<T> Result = bSuccess
					? Cast<T>(Texture)
					: nullptr;

				OnComplete(Result, Result != nullptr);
			});
		
		return;
	}

	FString GamePath = Path;

	Cloud::Export::GetAsyncOrSync(Path, {}, {}, [=](const TSharedPtr<FJsonObject>& Response) {
		if (!Response.IsValid() || Response->HasField(TEXT("errored"))) {
			OnComplete(nullptr, false);
			return;
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

			if (RootName != "Game" && RootName != "Engine" && GetPlugin(RootName) == nullptr) {
				CreatePlugin(RootName);
			}

			IImporter* OutImporter;
			bool bImportSuccess = IImportReader::ReadExportsAndImport(Response->GetArrayField(TEXT("exports")), PackagePath, OutImporter, true);

			if (!bImportSuccess) {
				OnComplete(nullptr, false);
				return;
			}
			
			/* Define found object */
			FString RedirectedPath = RealPath;
			FJRedirects::Redirect(RedirectedPath);

			UObject* Loaded = StaticLoadObject(T::StaticClass(), nullptr, *RedirectedPath);

			TObjectPtr<T> Result = Cast<T>(Loaded);
			
			OnComplete(Result, Result != nullptr);
		}
	}, false);
}

void FAssetUtilities::CreateTexture(const FString& Path, const FString& FetchPath, const TFunction<void(UTexture*, bool)>& OnComplete) {
	if (Path.IsEmpty()) {
		OnComplete(nullptr, false);
		return;
	}

	Cloud::Export::GetRaw(FetchPath, {}, {}, [=](const TSharedPtr<FJsonObject>& JsonObject) {
		if (JsonObject == nullptr) {
			OnComplete(nullptr, false);
			return;
		}

		TArray<TSharedPtr<FJsonValue>> Response = JsonObject->GetArrayField(TEXT("exports"));
		if (Response.Num() == 0) {
			OnComplete(nullptr, false);
			return;
		}

		const TSharedPtr<FJsonObject> JsonExport = Response[0]->AsObject();
		const FString Type = JsonExport->GetStringField(TEXT("Type"));

		bool IsVectorDisplacementMap = false;

		if (JsonExport->HasField(TEXT("Properties"))) {
			if (JsonExport->GetObjectField(TEXT("Properties"))->HasField(TEXT("CompressionSettings"))) {
				IsVectorDisplacementMap =
					JsonExport->GetObjectField(TEXT("Properties"))->GetStringField(TEXT("CompressionSettings")).Contains("TC_VectorDisplacementmap")
					|| JsonExport->GetObjectField(TEXT("Properties"))->GetStringField(TEXT("CompressionSettings")).Contains("TC_HDR");
			}
		}
		
		TArray<uint8> Data = TArray<uint8>();
		bool UseOctetStream = Type == "TextureLightProfile"
		                       || Type == "TextureCube"
		                       || Type == "VolumeTexture"
		                       || Type == "TextureRenderTarget2D" || IsVectorDisplacementMap;

	#if UE4_26_BELOW
		UseOctetStream = true;
	#endif

	#if UE5_5_BEYOND
		UseOctetStream = true;
	#endif

		/* ~~~~~~~~~~~~~~~ Download Texture Data ~~~~~~~~~~~~ */
		if (Type != "TextureRenderTarget2D") {
			FHttpModule* HttpModule = &FHttpModule::Get();
			const auto HttpRequest = HttpModule->CreateRequest();

			HttpRequest->SetURL(Cloud::URL + "/api/export?path=" + FetchPath);
			HttpRequest->SetHeader("content-type", UseOctetStream ? "application/octet-stream" : "image/png");
			HttpRequest->SetVerb(TEXT("GET"));
			
			const auto HttpResponse = FRemoteUtilities::ExecuteRequestSync(HttpRequest);

			if (!HttpResponse.IsValid() || HttpResponse->GetResponseCode() != 200) {
				OnComplete(nullptr, false);
				return;	
			}

			if (HttpResponse->GetContentType().StartsWith("application/json; charset=utf-8")) {
				OnComplete(nullptr, false);
				return;	
			}

			Data = HttpResponse->GetContent();
			if (Data.Num() == 0) {
				OnComplete(nullptr, false);
				return;	
			}
		}

		Fast_CreateTexture(JsonExport, Path, Type, Data, OnComplete);
	});
}

void FAssetUtilities::Fast_CreateTexture(const TSharedPtr<FJsonObject>& JsonExport, const FString& Path, const FString& Type, TArray<uint8> Data, TFunction<void(UTexture*, bool)> OnComplete) {
	const UJsonAsAssetSettings* Settings = GetSettings();
	UTexture* Texture = nullptr;
	
	FString PackagePath;
	FString AssetName; {
		Path.Split(".", &PackagePath, &AssetName);
	}

	bool IsVectorDisplacementMap = false;

	if (JsonExport->HasField(TEXT("Properties"))) {
		if (JsonExport->GetObjectField(TEXT("Properties"))->HasField(TEXT("CompressionSettings"))) {
			IsVectorDisplacementMap = JsonExport->GetObjectField(TEXT("Properties"))->GetStringField(TEXT("CompressionSettings")).Contains("TC_VectorDisplacementmap")
				|| JsonExport->GetObjectField(TEXT("Properties"))->GetStringField(TEXT("CompressionSettings")).Contains("TC_HDR");
		}
	}
	
	bool UseOctetStream = Type == "TextureLightProfile"
						   || Type == "TextureCube"
						   || Type == "VolumeTexture"
						   || Type == "TextureRenderTarget2D" || IsVectorDisplacementMap;

#if UE4_26_BELOW
	UseOctetStream = true;
#endif

#if UE5_5_BEYOND
	UseOctetStream = true;
#endif
	
	FJRedirects::Redirect(PackagePath);

	if (!PackagePath.StartsWith("/Game/") && !PackagePath.StartsWith("/Engine/")) {
		FString PluginName;
		PackagePath.Split("/", nullptr, &PluginName, ESearchCase::IgnoreCase, ESearchDir::FromStart);
		PluginName.Split("/", &PluginName, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromStart);

		if (GetPlugin(PluginName) == nullptr) {
			CreatePlugin(PluginName);
		}
	}
	
	UPackage* Package = CreateAssetPackage(*PackagePath);
	Package->FullyLoad();

	FTextureCreatorUtilities TextureCreator = FTextureCreatorUtilities(AssetName, Path, Package, UseOctetStream);

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
		OnComplete(nullptr, false);
		return;	
	}

	FAssetRegistryModule::AssetCreated(Texture);
	if (!Texture->MarkPackageDirty()) {
		OnComplete(nullptr, false);
		return;	
	}

	Package->SetDirtyFlag(true);
	Texture->PostEditChange();
	Texture->AddToRoot();
	Package->FullyLoad();

	/* Save texture */
	if (Settings->AssetSettings.SaveAssets) {
		SavePackage(Package);
	}

	OnComplete(Texture, true);
}