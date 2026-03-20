/* Copyright JsonAsAsset Contributors 2024-2026 */

#pragma once

#include "Json.h"
#include "Dom/JsonObject.h"
#include "UObject/Object.h"
#include "Engine/Compatibility.h"

/* A structure to hold data for a UObject export. */
struct FUObjectExport : FUObjectJsonValueExport {
	FUObjectExport(): Object(nullptr), Parent(nullptr), Package(nullptr), Position(-1) { };
	
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

	void SetParent(UObject* NewParent) {
		Parent = NewParent;
	}

	void SetObject(UObject* NewObject) {
		Object = NewObject;
	}

	void SetPosition(const int NewPosition) {
		Position = NewPosition;
	}

	explicit FUObjectExport(const TSharedPtr<FJsonObject>& InJsonObject)
		: FUObjectJsonValueExport(InJsonObject), Object(nullptr), Parent(nullptr), Position(-1) { }
	
	FUObjectExport(const TSharedPtr<FJsonObject>& InJsonObject, UObject* Object, UObject* Parent, const int Position = -1)
		: FUObjectJsonValueExport(InJsonObject), Object(Object), Parent(Parent), Position(Position) { }

	FUObjectExport(const FName OuterOverride, const TSharedPtr<FJsonObject>& InJsonObject, UObject* Object, UObject* Parent, const int Position = -1)
		: FUObjectJsonValueExport(InJsonObject), OuterOverride(OuterOverride), Object(Object), Parent(Parent), Position(Position) { }

	FUObjectExport(const FName NameOverride, const FName TypeOverride, const FName OuterOverride,
		const TSharedPtr<FJsonObject>& InJsonObject, UObject* Object, UObject* Parent, const int Position = -1)
		: FUObjectJsonValueExport(InJsonObject),
		  Object(Object),
		  Parent(Parent),
		  Position(Position),
		  NameOverride(NameOverride),
		  TypeOverride(TypeOverride),
		  OuterOverride(OuterOverride) { }
	
	const TSharedPtr<FJsonObject>& GetProperties() const {
		return JsonObject->GetObjectField(TEXT("Properties"));
	}

	FUObjectJsonValueExport GetPropertiesAsValue() const {
		return FUObjectJsonValueExport(GetProperties());
	}

	FUObjectJsonValueExport AsValueExport() const {
		return FUObjectJsonValueExport(JsonObject);
	}

	FUObjectJsonValueExport GetJsonObject() const {
		return FUObjectJsonValueExport(JsonObject);
	}
	
	UClass* GetClass() {
		if (Class) return Class;
		
		UClass* ClassRef = FindClassByType(GetType().ToString());
		if (ClassRef == nullptr) return nullptr;
		
		Class = ClassRef;
		return Class;
	}
	
	FName NameOverride;

	FName GetName() const {
		if (!NameOverride.IsNone()) {
			return NameOverride;
		}
		
		if (!JsonObject.IsValid() || !JsonObject->HasField(TEXT("Name"))) {
			return "";
		}

		return FName(*JsonObject->GetStringField(TEXT("Name")));
	}

	FName TypeOverride;

	FName GetType() const {
		if (!TypeOverride.IsNone()) {
			return TypeOverride;
		}
		
		if (!JsonObject.IsValid() || !JsonObject->HasField(TEXT("Type"))) {
			return "";
		}
		
		return FName(*JsonObject->GetStringField(TEXT("Type")));
	}

	FName OuterOverride;

	FName GetOuter() const {
		if (!OuterOverride.IsNone()) {
			return OuterOverride;
		}

		if (!JsonObject.IsValid() || !JsonObject->HasField(TEXT("Outer"))) {
			return "";
		}
		
		return FName(*JsonObject->GetStringField(TEXT("Outer")));
	}

	FName PathOverride;

	FName GetPath() const {
		if (!PathOverride.IsNone()) {
			return PathOverride;
		}

		if (!JsonObject.IsValid() || !JsonObject->HasField(TEXT("Path"))) {
			return "";
		}
		
		return FName(*JsonObject->GetStringField(TEXT("Path")));
	}

	TArray<FName> GetPathSegments(const bool bRemoveLast = false) const {
		TArray<FName> Result;
		const FString FullPath = GetPath().ToString();

		int32 ColonIndex = INDEX_NONE;
		if (!FullPath.FindChar(TEXT(':'), ColonIndex)) {
			return Result;
		}

		FString AfterColon = FullPath.Mid(ColonIndex + 1);

		TArray<FString> Parts;
		AfterColon.ParseIntoArray(Parts, TEXT("."), true);

		for (const FString& Part : Parts) {
			Result.Add(FName(*Part));
		}

		if (bRemoveLast && Result.Num() > 0) {
			Result.Pop(false);
		}

		return Result;
	}

	bool IsJsonAndObjectValid() const {
		return JsonObject != nullptr && Object != nullptr;
	}

	bool IsJsonValid() const {
		return JsonObject != nullptr && this != &EmptyExport();
	}

	bool IsJsonInvalid() const {
		return !IsJsonValid();
	}

	static FUObjectExport& EmptyExport() {
		static FUObjectExport Empty;
		return Empty;
	}

	explicit operator bool() const {
		return IsJsonValid();
	}

protected:
	UClass* Class = nullptr;
};