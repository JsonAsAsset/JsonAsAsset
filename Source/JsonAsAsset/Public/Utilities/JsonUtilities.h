/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

/* Conversion Functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#if ENGINE_UE5
inline FVector3f ObjectToVector3F(const FJsonObject* Object) {
	return FVector3f(Object->GetNumberField(TEXT("X")), Object->GetNumberField(TEXT("Y")), Object->GetNumberField(TEXT("Z")));
}
#else

inline FVector ObjectToVector3F(const FJsonObject* Object) {
	return FVector(Object->GetNumberField(TEXT("X")), Object->GetNumberField(TEXT("Y")), Object->GetNumberField(TEXT("Z")));
}
#endif

inline FLinearColor ObjectToLinearColor(const FJsonObject* Object) {
	return FLinearColor(Object->GetNumberField(TEXT("R")), Object->GetNumberField(TEXT("G")), Object->GetNumberField(TEXT("B")), Object->GetNumberField(TEXT("A")));
}

inline FRichCurveKey ObjectToRichCurveKey(const TSharedPtr<FJsonObject>& Object) {
	const FString InterpMode = Object->GetStringField(TEXT("InterpMode"));
	
	return FRichCurveKey(Object->GetNumberField(TEXT("Time")), Object->GetNumberField(TEXT("Value")), Object->GetNumberField(TEXT("ArriveTangent")), Object->GetNumberField(TEXT("LeaveTangent")), static_cast<ERichCurveInterpMode>(StaticEnum<ERichCurveInterpMode>()->GetValueByNameString(InterpMode)));
}
