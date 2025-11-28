/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"
#if ENGINE_UE5 && ENGINE_MINOR_VERSION >= 5
#include "StructUtils/UserDefinedStruct.h"
#else
#include "Engine/UserDefinedStruct.h"
#endif

class IUserDefinedStructImporter : public IImporter {
public:
	IUserDefinedStructImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& JsonObjects):
		IImporter(JsonObject, Package, JsonObjects) {
	}

	virtual UObject* CreateAsset(UObject* CreatedAsset) override;
	virtual bool Import() override;

protected:
	TSharedPtr<FJsonObject> CookedStructMetaData;
	TSharedPtr<FJsonObject> DefaultProperties;

	FEdGraphPinType ResolvePropertyPinType(const TSharedPtr<FJsonObject>& PropertyJsonObject);
	void ImportPropertyIntoStruct(UUserDefinedStruct* UserDefinedStruct, const TSharedPtr<FJsonObject>& PropertyJsonObject);
	UObject* LoadObjectFromJsonReference(const TSharedPtr<FJsonObject>& ParentJsonObject, const FString& ReferenceKey);
};

REGISTER_IMPORTER(IUserDefinedStructImporter, {
	TEXT("UserDefinedStruct")
}, "User Defined Assets");
