/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Utilities/Serializers/ObjectUtilities.h"
#include "Utilities/Compatibility.h"

#if ENGINE_UE5
#include "AnimGraphNode_Base.h"
#else
#include "AnimGraph/Classes/AnimGraphNode_Base.h"
#endif

#include "Utilities/Serializers/PropertyUtilities.h"
#include "UObject/Package.h"
#include "Utilities/EngineUtilities.h"

/* ReSharper disable once CppDeclaratorNeverUsed */
DECLARE_LOG_CATEGORY_CLASS(LogJsonAsAssetObjectSerializer, All, All);
PRAGMA_DISABLE_OPTIMIZATION

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

UObjectSerializer::UObjectSerializer(): Parent(nullptr), PropertySerializer(nullptr) {
}

void UObjectSerializer::SetupExports(const TArray<TSharedPtr<FJsonValue>>& InObjects) {
	Exports = InObjects;
}

UPackage* FindOrLoadPackage(const FString& PackageName) {
	UPackage* Package = FindPackage(nullptr, *PackageName);
	
	if (!Package) {
		Package = LoadPackage(nullptr, *PackageName, LOAD_None);
	}

	return Package;
}

void UObjectSerializer::SetPropertySerializer(UPropertySerializer* NewPropertySerializer) {
	check(NewPropertySerializer);

	this->PropertySerializer = NewPropertySerializer;
	NewPropertySerializer->ObjectSerializer = this;
}

void UObjectSerializer::SetExportForDeserialization(const TSharedPtr<FJsonObject>& JsonObject, UObject* Object) {
	ExportsToNotDeserialize.Add(JsonObject->GetStringField(TEXT("Name")));
	ConstructedObjects.Add(JsonObject->GetStringField(TEXT("Name")), Object);
}

void UObjectSerializer::DeserializeExports(TArray<TSharedPtr<FJsonValue>> InExports, const bool bCreateObjects) {
	PropertySerializer->ExportsContainer.Empty();

	int Index = -1;
	
	for (const TSharedPtr<FJsonValue> Object : InExports) {
		Index++;

		TSharedPtr<FJsonObject> ExportObject = Object->AsObject();

		/* No name = no export!! */
		if (!ExportObject->HasField(TEXT("Name"))) continue;

		FString Name = ExportObject->GetStringField(TEXT("Name"));
		FString Type = ExportObject->GetStringField(TEXT("Type"));
		
		/* Check if it's not supposed to be deserialized */
		if (ExportsToNotDeserialize.Contains(Name)) continue;
		if (Type == "NavCollision") continue;

		FString Outer = ExportObject->GetStringField(TEXT("Outer"));
		
		/* Add it to the referenced objects */
		PropertySerializer->ExportsContainer.Exports.Add(FUObjectExport(ExportObject, nullptr, Parent, Index));
	}

	if (bCreateObjects) {
		TMap<TSharedPtr<FJsonObject>, UObject*> ExportsMap;
		
		for (FUObjectExport& Export : PropertySerializer->ExportsContainer.Exports) {
			DeserializeExport(Export, ExportsMap);
		}

		for (const auto Pair : ExportsMap) {
			TSharedPtr<FJsonObject> Properties = Pair.Key;
			UObject* Object = Pair.Value;

			DeserializeObjectProperties(Properties, Object);
		}
	}
}

void UObjectSerializer::DeserializeExport(FUObjectExport& Export, TMap<TSharedPtr<FJsonObject>, UObject*>& ExportsMap) {
	if (Export.Object != nullptr) return;

	const TSharedPtr<FJsonObject> ExportObject = Export.JsonObject;

	/* No name means no export */
	if (!ExportObject->HasField(TEXT("Name"))) return;

	const FString Name = ExportObject->GetStringField(TEXT("Name"));
	const FString Type = ExportObject->GetStringField(TEXT("Type")).Replace(TEXT("CommonWidgetSwitcher"), TEXT("CommonActivatableWidgetSwitcher"));
		
	/* Check if it's not supposed to be deserialized */
	if (ExportsToNotDeserialize.Contains(Name)) return;
	if (Type == "BodySetup" || Type == "NavCollision") return;

	FString ClassName = ExportObject->GetStringField(TEXT("Class"));

	if (ExportObject->HasField(TEXT("Template"))) {
		const TSharedPtr<FJsonObject> TemplateObject = ExportObject->GetObjectField(TEXT("Template"));
		ClassName = ReadPathFromObject(&TemplateObject).Replace(TEXT("Default__"), TEXT(""));
	}
	
	if (ClassName.Contains("'")) {
		ClassName.Split("'", nullptr, &ClassName, ESearchCase::IgnoreCase, ESearchDir::FromStart);
		ClassName.Split("'", &ClassName, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromStart);
	}
	
	const UClass* Class = FindClassByType(ClassName);
	
	if (!Class) {
		Class = FindClassByType(Type);
	}

	if (!Class) return;

	const FString Outer = ExportObject->GetStringField(TEXT("Outer"));
	UObject* ObjectOuter = nullptr;

	if (FUObjectExport& FoundExport = PropertySerializer->ExportsContainer.Find(Outer); FoundExport.JsonObject.IsValid()) {
		if (FoundExport.Object == nullptr) {
			DeserializeExport(FoundExport, ExportsMap);
		}
		
		UObject* FoundObject = FoundExport.Object;
		ObjectOuter = FoundObject;
	}

	if (UObject** ConstructedObject = ConstructedObjects.Find(Outer)) {
		ObjectOuter = *ConstructedObject;
	}

	if (PathsToNotDeserialize.Contains(Outer + "." + Name)) return;
	if (ObjectOuter == nullptr) {
		ObjectOuter = Parent;
	}

	UObject* NewUObject = NewObject<UObject>(ObjectOuter, Class, FName(*Name));

	if (ExportObject->HasField(TEXT("Properties"))) {
		const TSharedPtr<FJsonObject> Properties = ExportObject->GetObjectField(TEXT("Properties"));

		ExportsMap.Add(Properties, NewUObject);
	} else {
		ExportsMap.Add(ExportObject, NewUObject);
	}

	/* Add it to the referenced objects */
	Export.Object = NewUObject;

	/* Already deserialized */
	PathsToNotDeserialize.Add(Outer + "." + Name);
}

void UObjectSerializer::DeserializeObjectProperties(const TSharedPtr<FJsonObject>& Properties, UObject* Object) const {
	if (Object == nullptr) return;

	const UClass* ObjectClass = Object->GetClass();

	for (FProperty* Property = ObjectClass->PropertyLink; Property; Property = Property->PropertyLinkNext) {
		const FString PropertyName = Property->GetName();

		if (!PropertySerializer->ShouldDeserializeProperty(Property)) continue;

		void* PropertyValue = Property->ContainerPtrToValuePtr<void>(Object);
		const bool HasHandledProperty = PassthroughPropertyHandler(Property, PropertyName, PropertyValue, Properties, PropertySerializer);

		/* Handler Specifically for Animation Blueprint Graph Nodes */
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property)) {
			if (StructProperty->Struct->IsChildOf(FAnimNode_Base::StaticStruct())) {
				void* StructPtr = StructProperty->ContainerPtrToValuePtr<void>(Object);

				if (static_cast<FAnimNode_Base*>(StructPtr)) {
					PropertySerializer->DeserializeStruct(StructProperty->Struct, Properties.ToSharedRef(), PropertyValue);
				}
			}
		}
		
		if (Properties->HasField(PropertyName) && !HasHandledProperty && PropertyName != "LODParentPrimitive" && PropertyName != "bIsCookedForEditor") {
			const TSharedPtr<FJsonValue>& ValueObject = Properties->Values.FindChecked(PropertyName);

			if (Property->ArrayDim == 1 || ValueObject->Type == EJson::Array) {
				PropertySerializer->DeserializePropertyValue(Property, ValueObject.ToSharedRef(), PropertyValue);
			}
		}
	}

	/* This is a use case for importing maps and parsing static mesh components
	 * using the object and property serializer, this was initially wanted to be
	 * done completely without any manual work. (using the de-serializers)
	 * However I don't think it's possible to do so. as I haven't seen any native
	 * property that can do this using the data provided in CUEParse.
	 */
	if (Properties->HasField(TEXT("LODData")) && Cast<UStaticMeshComponent>(Object)) {
		UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Object);
		if (!StaticMeshComponent) return;
		
		TArray<TSharedPtr<FJsonValue>> ObjectLODData = Properties->GetArrayField(TEXT("LODData"));
		int CurrentLOD = -1;
		
		for (const TSharedPtr CurrentLODValue : ObjectLODData) {
			CurrentLOD++;

			const TSharedPtr<FJsonObject> CurrentLODObject = CurrentLODValue->AsObject();

			/* Must contain vertex colors, or else it's an empty LOD */
			if (!CurrentLODObject->HasField(TEXT("OverrideVertexColors"))) continue;

			const TSharedPtr<FJsonObject> OverrideVertexColorsObject = CurrentLODObject->GetObjectField(TEXT("OverrideVertexColors"));

			if (!OverrideVertexColorsObject->HasField(TEXT("Data"))) continue;

			const int32 NumVertices = OverrideVertexColorsObject->GetIntegerField(TEXT("NumVertices"));
			const TArray<TSharedPtr<FJsonValue>> DataArray = OverrideVertexColorsObject->GetArrayField(TEXT("Data"));

			/* Template of the target data */
			FString Output = FString::Printf(TEXT("CustomLODData LOD=%d, ColorVertexData(%d)=("), CurrentLOD, NumVertices);

			/* Append the colors in the expected format */
			for (int32 i = 0; i < DataArray.Num(); ++i) {
				FString ColorValue = DataArray.operator[](i)->AsString();
				Output.Append(ColorValue);

				/* Add a comma unless it's the last element */
				if (i < DataArray.Num() - 1) {
					Output.Append(TEXT(","));
				}
			}

			Output.Append(TEXT(")"));
		
			StaticMeshComponent->ImportCustomProperties(*Output, GWarn);
		}
	}
}

PRAGMA_ENABLE_OPTIMIZATION