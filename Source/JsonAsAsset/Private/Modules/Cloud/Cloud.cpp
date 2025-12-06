/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Modules/Cloud/Cloud.h"

#include "HttpModule.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Settings/JsonAsAssetSettings.h"
#include "Settings/Runtime.h"
#include "Utilities/EngineUtilities.h"
#include "Utilities/RemoteUtilities.h"

TSharedPtr<FJsonObject> Cloud::Export::Get(const TMap<FString, FString>& Parameters, const TMap<FString, FString>& Headers) {
	return Cloud::Get(ExportURL, Parameters, Headers);
}

TSharedPtr<FJsonObject> Cloud::Export::Get(const FString& Path, const bool Raw, TMap<FString, FString> Parameters, const TMap<FString, FString>& Headers) {
	Parameters.Add(TEXT("path"), Path);
	Parameters.Add(TEXT("raw"), Raw ? TEXT("true") : TEXT("false"));
	
	return Get(Parameters, Headers);
}

TSharedPtr<FJsonObject> Cloud::Export::GetRaw(const FString& Path, const TMap<FString, FString>& Parameters, const TMap<FString, FString>& Headers) {
	return Get(Path, true, Parameters, Headers);
}

TArray<TSharedPtr<FJsonValue>> Cloud::Export::Array::Get(const TMap<FString, FString>& Parameters, const TMap<FString, FString>& Headers) {
	const TSharedPtr<FJsonObject> JsonObject = Export::Get(Parameters, Headers);

	if (!JsonObject.IsValid()) return TArray<TSharedPtr<FJsonValue>>();
	
	return JsonObject->GetArrayField(TEXT("exports"));
}

TArray<TSharedPtr<FJsonValue>> Cloud::Export::Array::Get(const FString& Path, const bool Raw, TMap<FString, FString> Parameters, const TMap<FString, FString>& Headers) {
	Parameters.Add(TEXT("path"), Path);
	Parameters.Add(TEXT("raw"), Raw ? TEXT("true") : TEXT("false"));
	
	return Get(Parameters, Headers);
}

TArray<TSharedPtr<FJsonValue>> Cloud::Export::Array::GetRaw(const FString& Path, const TMap<FString, FString>& Parameters, const TMap<FString, FString>& Headers) {
	return Get(Path, true, Parameters, Headers);
}

auto Cloud::SendRequest(const FString& RequestURL, const TMap<FString, FString>& Parameters, const TMap<FString, FString>& Headers) {
	const auto NewRequest = HttpModule->CreateRequest();

	FString FullUrl = URL + RequestURL;
	
	if (Parameters.Num() > 0) {
		bool bFirst = true;

		for (const auto& Pair : Parameters) {
			FullUrl += bFirst ? TEXT("?") : TEXT("&");
			bFirst = false;

			FullUrl += FString::Printf(
				TEXT("%s=%s"),
				*FGenericPlatformHttp::UrlEncode(Pair.Key),
				*FGenericPlatformHttp::UrlEncode(Pair.Value)
			);
		}
	}

	for (const auto& Pair : Headers) {
		NewRequest->SetHeader(Pair.Key, Pair.Value);
	}
	
	NewRequest->SetURL(FullUrl);
	
	return NewRequest;
}

TSharedPtr<FJsonObject> Cloud::Get(const FString& RequestURL, const TMap<FString, FString>& Parameters, const TMap<FString, FString>& Headers) {
	const auto Request = SendRequest(RequestURL, Parameters, Headers);
	Request->SetVerb(TEXT("GET"));

	const auto Response = FRemoteUtilities::ExecuteRequestSync(Request);
	if (!Response.IsValid()) return TSharedPtr<FJsonObject>();

	if (!Response->GetHeader("Content-Type").Contains(TEXT("json"))) {
		return TSharedPtr<FJsonObject>();
	}

	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	
	if (TSharedPtr<FJsonObject> JsonObject; FJsonSerializer::Deserialize(JsonReader, JsonObject)) {
		if (Response->GetResponseCode() != 200) {
			return TSharedPtr<FJsonObject>();
		}
		
		return JsonObject;
	}

	return TSharedPtr<FJsonObject>();
}

TArray<uint8> Cloud::GetRaw(const FString& RequestURL, const TMap<FString, FString>& Parameters,
	const TMap<FString, FString>& Headers) {
	const auto Request = SendRequest(RequestURL, Parameters, Headers);
	Request->SetVerb(TEXT("GET"));

	const auto Response = FRemoteUtilities::ExecuteRequestSync(Request);
	if (!Response.IsValid()) return TArray<uint8>();

	return Response->GetContent();
}

TArray<TSharedPtr<FJsonValue>> Cloud::GetExports(const FString& RequestURL, const TMap<FString, FString>& Parameters) {
	const TSharedPtr<FJsonObject> JsonObject = Get(RequestURL, Parameters);
	if (!JsonObject.IsValid()) return {};

	return JsonObject->GetArrayField(TEXT("exports"));
}

void Cloud::Update() {
	UJsonAsAssetSettings* MutableSettings = GetSettings();
	if (!MutableSettings->bEnableCloudServer) return;
	
	const auto MetadataResponse = Get("/api/metadata");
	if (!MetadataResponse.IsValid()) return;
	
	if (MetadataResponse->HasField(TEXT("name"))) {
		FString Name = MetadataResponse->GetStringField(TEXT("name"));
		MutableSettings->AssetSettings.ProjectName = Name;
	}

	if (MetadataResponse->HasField(TEXT("major_version"))) {
		const int MajorVersion = MetadataResponse->GetIntegerField(TEXT("major_version"));

		GJsonAsAssetRuntime.MajorVersion = MajorVersion;
		GJsonAsAssetRuntime.bUE5Target = MajorVersion == 5;
	}

	if (MetadataResponse->HasField(TEXT("minor_version"))) {
		const int MinorVersion = MetadataResponse->GetIntegerField(TEXT("minor_version"));
			
		GJsonAsAssetRuntime.MinorVersion = MinorVersion;
	}

	if (MetadataResponse->HasField(TEXT("profile"))) {
		const auto Profile = MetadataResponse->GetObjectField(TEXT("profile"));

		GJsonAsAssetRuntime.Profile.Name = Profile->GetStringField(TEXT("name"));
	}

	SavePluginConfig(MutableSettings);
}
