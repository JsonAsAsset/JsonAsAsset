/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Dom/JsonObject.h"
#include "UObject/Object.h"
#include "Utilities/Compatibility.h"

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

protected:
	UClass* Class = nullptr;
};

struct FUObjectExportContainer {
	/* Array of Expression Exports */
	TArray<FUObjectExport> Exports;
	
	FUObjectExportContainer() {};

	void Fill(TArray<TSharedPtr<FJsonValue>> Array) {
		int Index = -1;
	
		for (const TSharedPtr Value : Array) {
			Index++;

			TSharedPtr<FJsonObject> Object = Value->AsObject();
			if (!Object->HasField(TEXT("Name"))) continue;

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

		static FUObjectExport Dummy;
		return Dummy;
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

	FUObjectExport Find(const FName Name, const FName Outer) {
		for (FUObjectExport& Export : Exports) {
			if (Export.GetName() == Name && Export.GetOuter() == Outer) {
				return Export;
			}
		}

		return FUObjectExport();
	}

	FUObjectExport Find(const int Position) {
		for (FUObjectExport& Export : Exports) {
			if (Export.Position == Position) {
				return Export;
			}
		}

		return FUObjectExport();
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

	FUObjectExport Find(const FString& Name, const FString& Outer) {
		return Find(FName(*Name), FName(*Outer));
	}

	FUObjectExport FindByType(const FName Type) {
		for (FUObjectExport& Export : Exports) {
			if (Export.GetType() == Type) {
				return Export;
			}
		}

		return FUObjectExport();
	}

	FUObjectExport FindByType(const FString& Type) {
		return FindByType(FName(*Type));
	}

	FUObjectExport FindByType(const FName Type, const FName Outer) {
		for (FUObjectExport& Export : Exports) {
			if (Export.GetType() == Type && Export.GetOuter() == Outer) {
				return Export;
			}
		}

		return FUObjectExport();
	}

	FUObjectExport FindByType(const FString& Type, const FString& Outer) {
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
};