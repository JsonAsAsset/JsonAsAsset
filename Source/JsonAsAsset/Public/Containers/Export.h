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

	FUObjectExport GetExport(const FUObjectJsonValueExport& PackageIndex) {
		FString ObjectName = PackageIndex.GetString("ObjectName"); /* Class'Asset:ExportName' */
		FString ObjectPath = PackageIndex.GetString("ObjectPath"); /* Path/Asset.Index */
		FString Outer;
	
		/* Clean up ObjectName (Class'Asset:ExportName' --> Asset:ExportName --> ExportName) */
		ObjectName.Split("'", nullptr, &ObjectName);
		ObjectName.Split("'", &ObjectName, nullptr);

		if (ObjectName.Contains(":")) {
			ObjectName.Split(":", nullptr, &ObjectName); /* Asset:ExportName --> ExportName */
		}

		if (ObjectName.Contains(".")) {
			ObjectName.Split(".", nullptr, &ObjectName);
		}

		if (ObjectName.Contains(".")) {
			ObjectName.Split(".", &Outer, &ObjectName);
		}

		int Index = 0;

		/* Search for the object in the JsonObjects array */
		for (auto& Value : Exports) {
			FString Name;
			if (Value.JsonObject->TryGetStringField(TEXT("Name"), Name) && Name == ObjectName) {
				if (Value.JsonObject->HasField(TEXT("Outer")) && !Outer.IsEmpty()) {
					FString OuterName = Value.JsonObject->GetStringField(TEXT("Outer"));

					if (OuterName == Outer) {
						return Value;
					}
				} else {
					return Value;
				}
			}

			Index++;
		}

		return FUObjectExport::EmptyExport();
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

	FUObjectExport& GetExportStartingWith(const FString& PropertyName, const FString& Name) {
		for (FUObjectExport& Export : Exports) {
			if (Export.GetString(PropertyName).StartsWith(Name)) {
				return Export;
			}
		}

		return FUObjectExport::EmptyExport();
	}

	FUObjectExport& GetExportByObjectPath(const FUObjectJsonValueExport& JsonExport) {
		return GetExportByObjectPath(JsonExport.JsonObject);
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

	FUObjectExport FindByPosition(const int Position) {
		for (const FUObjectExport& Export : Exports) {
			if (Export.Position == Position) {
				return Export;
			}
		}

		return FUObjectExport::EmptyExport();
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

	FUObjectExport& FindBySegment(const TArray<FName>& Segments) {
    	for (FUObjectExport& Export : Exports) {
    		if (Export.GetPathSegments() == Segments) {
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

	/* Iterate exports, then execute lambda */
	template<typename FuncType>
	void ExportsLoop(const TArray<FUObjectJsonValueExport>& Exports, FuncType&& Func) {
		for (const FUObjectJsonValueExport& Export : Exports) {
			FUObjectExport& DirectExport = GetExportByObjectPath(Export);

			if (!DirectExport.IsJsonValid() || &DirectExport == &FUObjectExport::EmptyExport()) {
				continue;
			}

			Func(DirectExport);
		}
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