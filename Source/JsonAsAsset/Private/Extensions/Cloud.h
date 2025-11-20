/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Utilities/RemoteUtilities.h"

/* TODO: Implement this into the rest of JsonAsAsset */
class Cloud {
public:
	static inline FString URL = TEXT("http://localhost:1500");
	static inline FString Export = TEXT("/api/export");

public:
	static auto SendRequest(const FString& RequestURL, const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {}) {
		FHttpModule* HttpModule = &FHttpModule::Get();
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
	
	static TSharedPtr<FJsonObject> Get(const FString& RequestURL, const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {}) {
		const auto Request = SendRequest(RequestURL, Parameters, Headers);
		Request->SetVerb(TEXT("GET"));
	
		const auto Response = FRemoteUtilities::ExecuteRequestSync(Request);
		if (!Response.IsValid()) return TSharedPtr<FJsonObject>();

		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		
		if (TSharedPtr<FJsonObject> JsonObject; FJsonSerializer::Deserialize(JsonReader, JsonObject)) {
			return JsonObject;
		}

		return TSharedPtr<FJsonObject>();
	}

	static TArray<uint8> GetRaw(const FString& RequestURL, const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {}) {
		const auto Request = SendRequest(RequestURL, Parameters, Headers);
		Request->SetVerb(TEXT("GET"));
	
		const auto Response = FRemoteUtilities::ExecuteRequestSync(Request);
		if (!Response.IsValid()) return TArray<uint8>();

		return Response->GetContent();
	}

	static TArray<TSharedPtr<FJsonValue>> GetExports(const FString& RequestURL, const TMap<FString, FString>& Parameters = {}) {
		const TSharedPtr<FJsonObject> JsonObject = Get(RequestURL, Parameters);
		if (!JsonObject.IsValid()) return {};

		return JsonObject->GetArrayField(TEXT("exports"));
	}
};