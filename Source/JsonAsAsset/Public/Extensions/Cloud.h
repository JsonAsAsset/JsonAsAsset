/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Serialization/JsonSerializer.h"

class JSONASASSET_API Cloud {
public:
	static inline FString URL = TEXT("http://localhost:1500");
	static inline FHttpModule* HttpModule = &FHttpModule::Get();

	class JSONASASSET_API Export {
	public:
		static TSharedPtr<FJsonObject> Get(const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {});
		static TSharedPtr<FJsonObject> Get(const FString& Path, const bool Raw, TMap<FString, FString> Parameters = {}, const TMap<FString, FString>& Headers = {});
		static TSharedPtr<FJsonObject> GetRaw(const FString& Path, const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {});
	};

	class JSONASASSET_API Exports {
	public:
		static TArray<TSharedPtr<FJsonValue>> Get(const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {});
		static TArray<TSharedPtr<FJsonValue>> Get(const FString& Path, const bool Raw, TMap<FString, FString> Parameters = {}, const TMap<FString, FString>& Headers = {});
		static TArray<TSharedPtr<FJsonValue>> GetRaw(const FString& Path, const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {});
	};

public:
	static inline FString ExportURL = TEXT("/api/export");

public:
	static auto SendRequest(const FString& RequestURL, const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {});
	static TSharedPtr<FJsonObject> Get(const FString& RequestURL, const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {});
	static TArray<uint8> GetRaw(const FString& RequestURL, const TMap<FString, FString>& Parameters = {}, const TMap<FString, FString>& Headers = {});
	static TArray<TSharedPtr<FJsonValue>> GetExports(const FString& RequestURL, const TMap<FString, FString>& Parameters = {});
};