/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Utilities/Serializers/SerializerContainer.h"

/* Serializer Constructor */
USerializerContainer::USerializerContainer(UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& JsonObjects) : JsonObjects(JsonObjects) {
	AssetExport.Package = Package;
	AssetContainer.Fill(JsonObjects);
	
	CreateSerializer();
}

USerializerContainer::USerializerContainer() : USerializerContainer(nullptr, {}) { }

UObjectSerializer* USerializerContainer::GetObjectSerializer() const {
	return ObjectSerializer;
}

UPropertySerializer* USerializerContainer::GetPropertySerializer() const {
	return GetObjectSerializer()->PropertySerializer;
}

void USerializerContainer::CreateSerializer() {
	ObjectSerializer = NewObject<UObjectSerializer>();
	GetObjectSerializer()->SetPropertySerializer(NewObject<UPropertySerializer>());
}

/* AssetExport ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~> */
FString USerializerContainer::GetAssetName() const {
	return AssetExport.GetName().ToString();
}

FString USerializerContainer::GetAssetType() const {
	return AssetExport.GetType().ToString();
}

TSharedPtr<FJsonObject> USerializerContainer::GetAssetData() const {
	return AssetExport.GetProperties();
}

TSharedPtr<FJsonObject>& USerializerContainer::GetAssetExport() {
	return AssetExport.JsonObject;
}

UClass* USerializerContainer::GetAssetClass() {
	return AssetExport.GetClass();
}

void USerializerContainer::SetParent(UObject* Parent) {
	AssetExport.Parent = Parent;
}

UObject* USerializerContainer::GetParent() const {
	return AssetExport.Parent;
}

UPackage* USerializerContainer::GetPackage() const {
	return AssetExport.Package;
}

void USerializerContainer::SetPackage(UPackage* NewPackage) {
	AssetExport.Package = NewPackage;
}
/* AssetExport <~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
