/* Copyright JsonAsAsset Contributors 2024-2026 */

#pragma once

#include "CoreMinimal.h"

#include "Distributions/DistributionFloat.h"
#include "Distributions/DistributionFloatConstant.h"
#include "Distributions/DistributionFloatConstantCurve.h"
#include "Distributions/DistributionFloatUniform.h"
#include "Distributions/DistributionFloatUniformCurve.h"
#include "Distributions/DistributionVector.h"
#include "Distributions/DistributionVectorConstant.h"
#include "Distributions/DistributionVectorConstantCurve.h"
#include "Distributions/DistributionVectorUniform.h"
#include "Distributions/DistributionVectorUniformCurve.h"

/*
 * This file is used to de-cook data back into editor data.
 * Used for particle systems only at the moment.
*/

struct FRawDistributionMemberAccessor : FRawDistribution {
	FDistributionLookupTable GetLookupTable() {
		return LookupTable;
	}
};

struct FDistributionDecookContext {
	bool IsFloat;

	UObject* Outer;

	FDistributionLookupTable LookupTable;

	FRawDistributionFloat RawDistributionFloat;
	FRawDistributionVector RawDistributionVector;

	FDistributionDecookContext(UObject* Outer, FRawDistribution& RawDistribution, bool bIsFloat) : IsFloat(bIsFloat), Outer(Outer) {
		FRawDistributionMemberAccessor Accessor;

		if (IsFloat) {
			RawDistributionFloat = *reinterpret_cast<FRawDistributionFloat*>(&RawDistribution);
			Accessor = *reinterpret_cast<FRawDistributionMemberAccessor*>(&RawDistributionFloat);
		} else {
			RawDistributionVector = *reinterpret_cast<FRawDistributionVector*>(&RawDistribution);
			Accessor = *reinterpret_cast<FRawDistributionMemberAccessor*>(&RawDistributionVector);
		}

		LookupTable = Accessor.GetLookupTable();
	}
	
	template<typename T>
	T* CreateDistribution() const {
		return NewObject<T>(Outer);
	}
};

/*
 * Checks if a lookup table is from a uniform distribution.
 * 
 * Side effect: One value uniforms are treated as constants.
 * (no difference in outcome) Reference: UDistributionFloatUniformCurve::GetOperation
 */
inline bool IsUniformTable(const FDistributionLookupTable& Table) {
	return Table.Op == RDO_Random || Table.Op == RDO_Extreme;
}

inline bool IsConstantUniformTable(const FDistributionLookupTable& Table) {
	return IsUniformTable(Table) && Table.EntryCount == 1;
}

inline FRawDistributionMemberAccessor& GetAccessor(FRawDistribution& RawDistribution) {
	return *reinterpret_cast<FRawDistributionMemberAccessor*>(&RawDistribution);
}

inline bool IsConstantDistribution(FDistributionDecookContext& Context) {
	if (Context.IsFloat) {
		float Min, Max; Context.RawDistributionFloat.GetOutRange(Min, Max);
		return Min == Max;
	}

	float Min, Max; Context.RawDistributionVector.GetOutRange(Min, Max);
	return Min == Max;
}

/************** */
inline UDistributionFloat* DecookFloatDistribution(FDistributionDecookContext& Context) {
	const FDistributionLookupTable& LookupTable = Context.LookupTable;

	if (LookupTable.Values.Num() == 0) return nullptr;

	/* Constant */
	if (IsConstantDistribution(Context)) {
		UDistributionFloatConstant* Distribution = Context.CreateDistribution<UDistributionFloatConstant>();
		
		Distribution->Constant = LookupTable.Values[0];
		
		return Distribution;
	}

	/* Uniform */
	if (IsConstantUniformTable(LookupTable)) {
		UDistributionFloatUniform* Distribution = Context.CreateDistribution<UDistributionFloatUniform>();
		
		Distribution->Min = LookupTable.Values[0];
		Distribution->Max = LookupTable.Values[1];
		
		return Distribution;
	}

	const float TimeScale = LookupTable.TimeScale == 0.f ? 0.f : 1.f / LookupTable.TimeScale;

	/* Uniform Curve */
	if (IsUniformTable(LookupTable)) {
		UDistributionFloatUniformCurve* Distribution = Context.CreateDistribution<UDistributionFloatUniformCurve>();
		
		for (int32 i = 0; i < LookupTable.EntryCount; i++) {
			const float Time = LookupTable.TimeBias + i * TimeScale;
			const float MinVal = LookupTable.Values[i * LookupTable.EntryStride];
			const float MaxVal = LookupTable.Values[
				i * LookupTable.EntryStride
				+ (LookupTable.SubEntryStride > 0 ? LookupTable.SubEntryStride : 1)
			];

			FInterpCurvePoint<FVector2D> Key;
			
			Key.InVal = Time;
			Key.OutVal = FVector2D(MinVal, MaxVal);
			Key.InterpMode = CIM_Linear;
			
			Distribution->ConstantCurve.Points.Add(Key);
		}
		
		return Distribution;
	}
	
	/* Constant Curve */
	UDistributionFloatConstantCurve* Distribution = Context.CreateDistribution<UDistributionFloatConstantCurve>();

	for (int32 i = 0; i < LookupTable.EntryCount; i++) {
		const float Time = LookupTable.TimeBias + i * TimeScale;
		const float Value = LookupTable.Values[i * LookupTable.EntryStride];

		FInterpCurvePoint<float> Key;
		
		Key.InVal = Time;
		Key.OutVal = Value;
		Key.InterpMode = CIM_Linear;
		
		Distribution->ConstantCurve.Points.Add(Key);
	}
	
	return Distribution;
}

inline UDistributionVector* DecookVectorDistribution(FDistributionDecookContext& Context) {
	const FDistributionLookupTable& LookupTable = Context.LookupTable;

	if (LookupTable.Values.Num() == 0) return nullptr;

	/* Constant */
	if (IsConstantDistribution(Context)) {
		UDistributionVectorConstant* Distribution = Context.CreateDistribution<UDistributionVectorConstant>();
		
		Distribution->Constant = FVector(
			LookupTable.Values[0],
			LookupTable.Values[1],
			LookupTable.Values[2]
		);
		
		return Distribution;
	}

	/* Uniform */
	if (IsConstantUniformTable(LookupTable)) {
		UDistributionVectorUniform* Distribution = Context.CreateDistribution<UDistributionVectorUniform>();
		
		Distribution->Min = FVector(
			LookupTable.Values[0],
			LookupTable.Values[1],
			LookupTable.Values[2]
		);
		
		Distribution->Max = FVector(
			LookupTable.Values[LookupTable.SubEntryStride],
			LookupTable.Values[LookupTable.SubEntryStride + 1],
			LookupTable.Values[LookupTable.SubEntryStride + 2]
		);
		
		return Distribution;
	}
	
	const float TimeScale = LookupTable.TimeScale == 0.f ? 0.f : 1.f / LookupTable.TimeScale;
	
	/* Uniform Curve */
	if (IsUniformTable(LookupTable)) {
		UDistributionVectorUniformCurve* Distribution = Context.CreateDistribution<UDistributionVectorUniformCurve>();
		
		for (int32 i = 0; i < LookupTable.EntryCount; i++) {
			const float Time = LookupTable.TimeBias + i * TimeScale;
			const int32 Base = i * LookupTable.EntryStride;

			FInterpCurvePoint<FTwoVectors> Key;
			Key.InVal = Time;
			
			Key.OutVal.v1 = FVector(
				LookupTable.Values[Base],
				LookupTable.Values[Base + 1],
				LookupTable.Values[Base + 2]
			);
			
			Key.OutVal.v2 = FVector(
				LookupTable.Values[Base + LookupTable.SubEntryStride],
				LookupTable.Values[Base + LookupTable.SubEntryStride + 1],
				LookupTable.Values[Base + LookupTable.SubEntryStride + 2]
			);
			
			Key.InterpMode = CIM_Linear;
			
			Distribution->ConstantCurve.Points.Add(Key);
		}
		
		return Distribution;
	}

	/* Constant Curve */
	UDistributionVectorConstantCurve* Distribution = Context.CreateDistribution<UDistributionVectorConstantCurve>();

	for (int32 i = 0; i < LookupTable.EntryCount; i++) {
		const float Time = LookupTable.TimeBias + i * TimeScale;
		const int32 Base = i * LookupTable.EntryStride;

		FInterpCurvePoint<FVector> Key;
		
		Key.InVal = Time;
		
		Key.OutVal = FVector(
			LookupTable.Values[Base],
			LookupTable.Values[Base + 1],
			LookupTable.Values[Base + 2]
		);
		
		Key.InterpMode = CIM_Linear;
		
		Distribution->ConstantCurve.Points.Add(Key);
	}
	
	return Distribution;
}

/************** Handlers */
inline UDistribution* DecookDistribution(UObject* Outer, FRawDistribution& RawDistribution, const bool IsFloat) {
	FDistributionDecookContext Context(Outer, RawDistribution, IsFloat);

	if (IsFloat) {
		return DecookFloatDistribution(Context);
	}

	/* Vector: By default */
	return DecookVectorDistribution(Context);
}