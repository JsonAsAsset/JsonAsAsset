/* Copyright JsonAsAsset Contributors 2024-2026 */

#pragma once

#include "Dom/JsonObject.h"
#include "UObject/Object.h"
#include "Utilities/Compatibility.h"

struct FUObjectJsonValueExport {
	FUObjectJsonValueExport(const TSharedPtr<FJsonValue>& Value): Value(Value) {
		if (Value->Type == EJson::Object) {
			JsonObject = Value->AsObject();
		}
	}

	FUObjectJsonValueExport(const TSharedPtr<FJsonObject>& JsonObject): JsonObject(JsonObject) {
		Value = MakeShared<FJsonValueObject>(JsonObject);
	}
	
	TSharedPtr<FJsonValue> Value;
	TSharedPtr<FJsonObject> JsonObject;
	
	/* Json Helpers */
	FString GetString(const FString& FieldName) const {
		return JsonObject->GetStringField(FieldName);
	}

	int32 GetInteger(const FString& FieldName) const {
		return JsonObject->GetIntegerField(FieldName);
	}

	bool GetBool(const FString& FieldName) const {
		return JsonObject->GetBoolField(FieldName);
	}

	double GetNumber(const FString& FieldName) const {
		return JsonObject->GetNumberField(FieldName);
	}

	FUObjectJsonValueExport GetObject(const FString& FieldName) const {
		return JsonObject->GetObjectField(FieldName);
	}

	TArray<FUObjectJsonValueExport> GetArray(const FString& FieldName) const {
		TArray<FUObjectJsonValueExport> Result;

		for (const TSharedPtr<FJsonValue>& Value : JsonObject->GetArrayField(FieldName)) {
			Result.Add(Value);
		}

		return Result;
	}
};

/* A structure to hold data for a UObject export. */
struct FUObjectExport {
	FUObjectExport(): Object(nullptr), Parent(nullptr), Position(-1) { };

	/* The json object of the expression, ^this is not Properties^ */
	TSharedPtr<FJsonObject> JsonObject;

	FName NameOverride;
	FName TypeOverride;
	FName OuterOverride;

	TSharedPtr<void> ExtraData;
	FName ExtraDataType;

	/* Object created */
	UObject* Object;

	template<typename T>
	T* Get() const {
		return Object ? Cast<T>(Object) : nullptr;
	}

	/* Parent of this expression */
	UObject* Parent;
	UPackage* Package;
	int Position;

	explicit FUObjectExport(const TSharedPtr<FJsonObject>& JsonObject)
		: JsonObject(JsonObject), Object(nullptr), Parent(nullptr), Position(-1) { }
	
	FUObjectExport(const TSharedPtr<FJsonObject>& JsonObject, UObject* Object, UObject* Parent, const int Position = -1)
		: JsonObject(JsonObject), Object(Object), Parent(Parent), Position(Position) { }

	FUObjectExport(const FName OuterOverride, const TSharedPtr<FJsonObject>& JsonObject, UObject* Object, UObject* Parent, const int Position = -1)
		: JsonObject(JsonObject), OuterOverride(OuterOverride), Object(Object), Parent(Parent), Position(Position) { }

	FUObjectExport(const FName NameOverride, const FName TypeOverride, const FName OuterOverride, const TSharedPtr<FJsonObject>& JsonObject, UObject* Object, UObject* Parent, int Position = -1)
		: JsonObject(JsonObject), NameOverride(NameOverride), TypeOverride(TypeOverride), OuterOverride(OuterOverride), Object(Object), Parent(Parent), Position(Position) { }

	const TSharedPtr<FJsonObject>& GetProperties() const {
		return JsonObject->GetObjectField(TEXT("Properties"));
	}
	
	UClass* GetClass() {
		if (Class) return Class;
		
		UClass* ClassRef = FindClassByType(GetType().ToString());
		if (ClassRef == nullptr) return nullptr;
		
		Class = ClassRef;
		return Class;
	}

	FName GetName() const {
		if (!NameOverride.IsNone()) {
			return NameOverride;
		}
		
		if (!JsonObject.IsValid() || !JsonObject->HasField(TEXT("Name"))) {
			return "";
		}

		return FName(*JsonObject->GetStringField(TEXT("Name")));
	}

	FName GetType() const {
		if (!TypeOverride.IsNone()) {
			return TypeOverride;
		}
		
		if (!JsonObject.IsValid() || !JsonObject->HasField(TEXT("Type"))) {
			return "";
		}
		
		return FName(*JsonObject->GetStringField(TEXT("Type")));
	}

	FName GetOuter() const {
		if (!OuterOverride.IsNone()) {
			return OuterOverride;
		}

		if (!JsonObject.IsValid() || !JsonObject->HasField(TEXT("Outer"))) {
			return "";
		}
		
		return FName(*JsonObject->GetStringField(TEXT("Outer")));
	}

	bool IsValid() const {
		return JsonObject != nullptr && Object != nullptr;
	}

	bool IsJsonValid() const {
		return JsonObject != nullptr;
	}

	static FUObjectExport& EmptyExport() {
		static FUObjectExport Empty;
		return Empty;
	}

protected:
	UClass* Class = nullptr;
};

struct FUObjectExportContainer {
	/* Array of Expression Exports */
	TArray<FUObjectExport> Exports;
	TArray<TSharedPtr<FJsonValue>> JsonObjects = { };
	
public:
	FUObjectExportContainer() { }
	FUObjectExportContainer(const TArray<TSharedPtr<FJsonValue>>& Array) {
		Fill(Array);
	}

	void Fill(TArray<TSharedPtr<FJsonValue>> Array) {
		JsonObjects = Array;
		
		int Index = -1;
	
		for (const TSharedPtr Value : Array) {
			Index++;

			TSharedPtr<FJsonObject> Object = Value->AsObject();
			if (!Object->HasField(TEXT("Name")) || !Object->HasField(TEXT("Type"))) continue;

			/* Add it to the referenced objects */
			Exports.Add(FUObjectExport(Object, nullptr, nullptr, Index));
		}
	}

	FUObjectExport& Find(const FName Name) {
		for (FUObjectExport& Export : Exports) {
			if (Export.GetName() == Name) {
				return Export;
			}
		}

		return FUObjectExport::EmptyExport();
	}

	template<typename T>
	T* Find(const FName Name) {
		for (FUObjectExport& Export : Exports) {
			if (Export.GetName() == Name) {
				return Export.Get<T>();
			}
		}

		return nullptr;
	}

	FUObjectExport& Find(const FName Name, const FName Outer) {
		for (FUObjectExport& Export : Exports) {
			if (Export.GetName() == Name && Export.GetOuter() == Outer) {
				return Export;
			}
		}

		return FUObjectExport::EmptyExport();
	}

	TArray<FUObjectExport> GetExportsWithPropertyNameStartingWith(const FString& PropertyName, const FString& StartingWith) {
		TArray<FUObjectExport> Result;
		
		for (FUObjectExport& Export : Exports) {
			if (Export.IsJsonValid() && Export.JsonObject->HasField(PropertyName)) {
				const FString TypeValue = Export.JsonObject->GetStringField(PropertyName);

				if (TypeValue.StartsWith(StartingWith)) {
					Result.Add(Export);
				}
			}
		}

		return Result;
	}

	TSharedPtr<FJsonObject> GetExportJsonObjectByObjectPath(const TSharedPtr<FJsonObject>& Object) {
		const TSharedPtr<FJsonObject> ValueObject = TSharedPtr(Object);

		FString IndexAsString; {
			ValueObject->GetStringField(TEXT("ObjectPath")).Split(".", nullptr, &IndexAsString);
		}

		for (FUObjectExport& Export : Exports) {
			if (Export.IsJsonValid()) {
				if (Export.Position == FCString::Atod(*IndexAsString)) {
					return Export.JsonObject;
				}
			}
		}

		return nullptr;
	}

	FUObjectExport& GetExportByObjectPath(const TSharedPtr<FJsonObject>& Object) {
		if (!Object.IsValid() || !Object->HasField(TEXT("ObjectPath"))) {
			return FUObjectExport::EmptyExport();
		}

		const FString ObjectPath = Object->GetStringField(TEXT("ObjectPath"));

		FString IndexString;
		if (!ObjectPath.Split(TEXT("."), nullptr, &IndexString)) {
			return FUObjectExport::EmptyExport();
		}

		const int32 Index = FCString::Atoi(*IndexString);

		for (FUObjectExport& Export : Exports) {
			if (Export.Position == Index) {
				return Export;
			}
		}

		return FUObjectExport::EmptyExport();
	}

	FUObjectExport& Find(const int Position) {
		for (FUObjectExport& Export : Exports) {
			if (Export.Position == Position) {
				return Export;
			}
		}

		return FUObjectExport::EmptyExport();
	}

	UObject* FindRef(const int Position) {
		for (const FUObjectExport& Export : Exports) {
			if (Export.Position == Position) {
				return Export.Object;
			}
		}

		return nullptr;
	}

	FUObjectExport& Find(const FString& Name) {
		return Find(FName(*Name));
	}

	FUObjectExport& Find(const FString& Name, const FString& Outer) {
		return Find(FName(*Name), FName(*Outer));
	}

	FUObjectExport& FindByType(const FName Type) {
		for (FUObjectExport& Export : Exports) {
			if (Export.GetType() == Type) {
				return Export;
			}
		}

		return FUObjectExport::EmptyExport();
	}

	FUObjectExport& FindByType(const FString& Type) {
		return FindByType(FName(*Type));
	}

	FUObjectExport& FindByType(const FName Type, const FName Outer) {
		for (FUObjectExport& Export : Exports) {
			if (Export.GetType() == Type && Export.GetOuter() == Outer) {
				return Export;
			}
		}

		return FUObjectExport::EmptyExport();
	}

	FUObjectExport& FindByType(const FString& Type, const FString& Outer) {
		return FindByType(FName(*Type), FName(*Outer));
	}
	
	bool Contains(const FName Name) {
		for (FUObjectExport& Export : Exports) {
			if (Export.GetName() == Name) {
				return true;
			}
		}

		return false;
	}

	void Empty() {
		Exports.Empty();
	}
	
	int Num() const {
		return Exports.Num();
	}

	auto begin() { return Exports.begin(); }
	auto end() { return Exports.end(); }

	auto begin() const { return Exports.begin(); }
	auto end() const { return Exports.end(); }
};