﻿/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

/*
 * This file is used to allow the same code used on UE5 to be used on UE4,
 * it contains structures and classes to replicate missing classes/structs.
 */

/* Compiles an experimental version of JsonAsAsset */
#ifndef JSONASASSET_EXPERIMENTAL
#define JSONASASSET_EXPERIMENTAL 0
#endif

#if ENGINE_MAJOR_VERSION == 5
	#define ENGINE_UE5 1
#else
	#define ENGINE_UE5 0
#endif

#if ENGINE_UE5 && ENGINE_MINOR_VERSION >= 6
	#define UE5_6_BEYOND 1
#else
	#define UE5_6_BEYOND 0
#endif

#if ENGINE_UE5 && ENGINE_MINOR_VERSION >= 6
	#define UE5_6_BEYOND 1
#else
	#define UE5_6_BEYOND 0
#endif

#if ENGINE_UE5 && ENGINE_MINOR_VERSION >= 3
	#define UE5_3_BEYOND 1
#else
	#define UE5_3_BEYOND 0
#endif

#if ENGINE_MAJOR_VERSION == 4
	#define ENGINE_UE4 1
#else
	#define ENGINE_UE4 0
#endif

#if ENGINE_UE4 && ENGINE_MINOR_VERSION == 26 && ENGINE_PATCH_VERSION == 0
	#define UE4_26_0 1
#else
	#define UE4_26_0 0
#endif

#if ENGINE_UE4 && ENGINE_MINOR_VERSION == 26
	#define UE4_26 1
#else
	#define UE4_26 0
#endif

#if ENGINE_UE4 && ENGINE_MINOR_VERSION <= 27
	#define UE4_27_BELOW 1
#else
	#define UE4_27_BELOW 0
#endif

#if ENGINE_UE4 && ENGINE_MINOR_VERSION < 26
	#define UE4_25_BELOW 1
#else
	#define UE4_25_BELOW 0
#endif

#if ENGINE_UE4 && ENGINE_MINOR_VERSION < 27
	#define UE4_27_ONLY_BELOW 1
#else
	#define UE4_27_ONLY_BELOW 0
#endif

#if ENGINE_UE4 && ENGINE_MINOR_VERSION <= 26
	#define UE4_26_BELOW 1
#else
	#define UE4_26_BELOW 0
#endif

#if ENGINE_UE5 && ENGINE_MINOR_VERSION >= 2
	#define UE5_2_BEYOND 1
#else
	#define UE5_2_BEYOND 0
#endif

#if ENGINE_UE5 && ENGINE_MINOR_VERSION < 2
	#define UE5_1_BELOW 1
#else
	#define UE5_1_BELOW 0
#endif

#if UE4_26_0
#include "AssetRegistry/Public/AssetRegistryModule.h"
#endif

#if ENGINE_UE5 || ((ENGINE_UE4 && ENGINE_MINOR_VERSION >= 26) && !(ENGINE_MINOR_VERSION == 26 && ENGINE_PATCH_VERSION == 0))
#include "AssetRegistry/AssetRegistryModule.h"
#endif

#if ENGINE_UE5
#include "Styling/AppStyle.h"
using FAppStyle = FAppStyle;
#else

#include "EditorStyleSet.h"

class FAppStyle {
public:
	static const ISlateStyle& Get() {
		return FEditorStyle::Get();
	}

	static FName GetAppStyleSetName() {
		return FEditorStyle::GetStyleSetName();
	}

	static const FSlateBrush* GetBrush(const FName PropertyName) {
		return FEditorStyle::GetBrush(PropertyName);
	}
};

template <typename TObjectType>
class TObjectPtr {
private:
	TWeakObjectPtr<TObjectType> WeakPtr;

public:
	TObjectPtr() {}
	// ReSharper disable once CppNonExplicitConvertingConstructor
	TObjectPtr(TObjectType* InObject) : WeakPtr(InObject) {}

	TObjectType* Get() const { return WeakPtr.Get(); }

	bool IsValid() const { return WeakPtr.IsValid(); }

	void Reset() { WeakPtr.Reset(); }

	void Set(TObjectType* InObject) { WeakPtr = InObject; }

	/* Additional constructor to allow raw pointer conversion */
	TObjectPtr(TObjectType* InObject, bool bRawPointer) : WeakPtr(InObject) {}

	/* Implicit conversion to raw pointer */
	// ReSharper disable once CppNonExplicitConversionOperator
	operator TObjectType*() const { return Get(); }

	/* Overload address-of operator */
	TObjectPtr<TObjectType>* operator&() { return this; }

	/* Assignment operator for TObjectType* */
	TObjectPtr& operator=(TObjectType* InObject) {
		WeakPtr = InObject;
		return *this;
	}

	// Comparison operator for nullptr
	bool operator==(std::nullptr_t) const { return Get() == nullptr; }
	bool operator!=(std::nullptr_t) const { return Get() != nullptr; }
};
#endif

template <typename T>
bool IsObjectPtrValid(TObjectPtr<T> ObjectPtr) {
#if ENGINE_UE5
	return ObjectPtr.Get() != nullptr;
#else
	return ObjectPtr.IsValid();
#endif
}

inline int32 GetElementSize(FProperty* Property) {
#if ENGINE_UE5 && UE5_6_BEYOND
	return Property->GetElementSize();
#else
	return Property->ElementSize;
#endif
}

inline const UObject* GetClassDefaultObject(UClass* Class) {
#if UE5_6_BEYOND
	return GetDefault<UObject>(Class);
#else
	return Class->ClassDefaultObject;
#endif
}