/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Dom/JsonObject.h"
#include "UObject/Object.h"

/* A structure to hold data for a UObject export. */
struct FUObjectExport {
	FUObjectExport(): Object(nullptr), Parent(nullptr), Position(-1) { };

	FName Name;
	FName Type;
	FName Outer;

	/* The json object of the expression, this is not Properties */
	TSharedPtr<FJsonObject> JsonObject;

	/* Object created */
	UObject* Object;

	template<typename T>
	T* Get() const {
		return Object ? Cast<T>(Object) : nullptr;
	}

	/* Parent of this expression */
	UObject* Parent;

	int Position;

	FUObjectExport(const FName& Name, const FName& Type, const FName Outer, const TSharedPtr<FJsonObject>& JsonObject, UObject* Object, UObject* Parent, int Position)
		: Name(Name), Type(Type), Outer(Outer), JsonObject(JsonObject), Object(Object), Parent(Parent), Position(Position) {
	}

	FUObjectExport(const FName& Name, const FName& Type, const FName Outer, const TSharedPtr<FJsonObject>& JsonObject, UObject* Object, UObject* Parent)
		: Name(Name), Type(Type), Outer(Outer), JsonObject(JsonObject), Object(Object), Parent(Parent), Position(-1) {
	}

	TSharedPtr<FJsonObject> GetProperties() const {
		TSharedPtr<FJsonObject> Properties = JsonObject->GetObjectField(TEXT("Properties"));

		return Properties;
	}

	bool IsValid() const {
		return JsonObject != nullptr && Object != nullptr;
	}

	bool IsJsonValid() const {
		return JsonObject != nullptr;
	}
};

struct FUObjectExportContainer {
	/* Array of Expression Exports */
	TArray<FUObjectExport> Exports;
	
	FUObjectExportContainer() {};

	FUObjectExport& Find(const FName Name) {
		for (FUObjectExport& Export : Exports) {
			if (Export.Name == Name) {
				return Export;
			}
		}

		static FUObjectExport Dummy;
		return Dummy;
	}

	template<typename T>
	T* Find(const FName Name) const {
		for (FUObjectExport Export : Exports) {
			if (Export.Name == Name) {
				return Export.Get<T>();
			}
		}

		return nullptr;
	}

	FUObjectExport Find(const FName Name, const FName Outer) {
		for (FUObjectExport Export : Exports) {
			if (Export.Name == Name && Export.Outer == Outer) {
				return Export;
			}
		}

		return FUObjectExport();
	}

	FUObjectExport Find(const int Position) {
		for (FUObjectExport Export : Exports) {
			if (Export.Position == Position) {
				return Export;
			}
		}

		return FUObjectExport();
	}

	UObject* FindRef(const int Position) {
		for (const FUObjectExport Export : Exports) {
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
		for (FUObjectExport Export : Exports) {
			if (Export.Type == Type) {
				return Export;
			}
		}

		return FUObjectExport();
	}

	FUObjectExport FindByType(const FString& Type) {
		return FindByType(FName(*Type));
	}

	FUObjectExport FindByType(const FName Type, const FName Outer) {
		for (FUObjectExport Export : Exports) {
			if (Export.Type == Type && Export.Outer == Outer) {
				return Export;
			}
		}

		return FUObjectExport();
	}

	FUObjectExport FindByType(const FString& Type, const FString& Outer) {
		return FindByType(FName(*Type), FName(*Outer));
	}
	
	bool Contains(const FName Name) {
		for (FUObjectExport Export : Exports) {
			if (Export.Name == Name) {
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