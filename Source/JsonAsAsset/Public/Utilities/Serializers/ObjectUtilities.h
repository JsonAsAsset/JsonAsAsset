/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "UObject/Object.h"
#include "Containers/ObjectExport.h"
#include "ObjectUtilities.generated.h"

class UPropertySerializer;

UCLASS()
class JSONASASSET_API UObjectSerializer : public UObject {
    GENERATED_BODY()
public:
    UObjectSerializer();

    void SetPropertySerializer(UPropertySerializer* NewPropertySerializer);
    void SetupExports(const TArray<TSharedPtr<FJsonValue>>& InObjects);

    FORCEINLINE UPropertySerializer* GetPropertySerializer() const { return PropertySerializer; }

    void DeserializeObjectProperties(const TSharedPtr<FJsonObject>& Properties, UObject* Object) const;

    void SetExportForDeserialization(const TSharedPtr<FJsonObject>& JsonObject, UObject* Object);
    void DeserializeExports(TArray<TSharedPtr<FJsonValue>> InExports, bool bCreateObjects = true);
    void DeserializeExport(FUObjectExport& Export, TMap<TSharedPtr<FJsonObject>, UObject*>& ExportsMap);

public:
    UPROPERTY()
    UObject* Parent;

    UPROPERTY()
    TMap<FString, UObject*> ConstructedObjects;
    
    UPROPERTY()
    UPropertySerializer* PropertySerializer;

    TArray<TSharedPtr<FJsonValue>> Exports;

    UPROPERTY()
    TArray<FString> ExportsToNotDeserialize;

    TArray<FString> PathsToNotDeserialize;
};