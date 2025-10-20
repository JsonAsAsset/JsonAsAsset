/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/Tables/CurveTableImporter.h"
#include "Dom/JsonObject.h"

/* TODO: Rewrite? */

void CCurveTableDerived::ChangeTableMode(const ECurveTableMode Mode) {
	CurveTableMode = Mode;
}

bool ICurveTableImporter::Import() {
	TSharedPtr<FJsonObject> RowData = AssetData->GetObjectField(TEXT("Rows"));
	UCurveTable* CurveTable = NewObject<UCurveTable>(Package, UCurveTable::StaticClass(), *AssetName, RF_Public | RF_Standalone);
	CCurveTableDerived* DerivedCurveTable = Cast<CCurveTableDerived>(CurveTable);

	/* Used to determine curve type */
	ECurveTableMode CurveTableMode = ECurveTableMode::RichCurves; {
		if (FString CurveMode; AssetData->TryGetStringField(TEXT("CurveTableMode"), CurveMode))
			CurveTableMode = static_cast<ECurveTableMode>(StaticEnum<ECurveTableMode>()->GetValueByNameString(CurveMode));

		DerivedCurveTable->ChangeTableMode(CurveTableMode);
	}

	/* Loop throughout row data, and deserialize */
	for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : RowData->Values) {
		const TSharedPtr<FJsonObject> CurveData = Pair.Value->AsObject();

		/* Curve structure (either simple or rich) */
		FRealCurve RealCurve;

		if (CurveTableMode == ECurveTableMode::RichCurves) {
			FRichCurve& NewRichCurve = CurveTable->AddRichCurve(FName(*Pair.Key)); {
				RealCurve = static_cast<FRealCurve>(NewRichCurve);
			}

			if (const TArray<TSharedPtr<FJsonValue>>* KeysPtr; CurveData->TryGetArrayField(TEXT("Keys"), KeysPtr))
				for (const TSharedPtr<FJsonValue> KeyPtr : *KeysPtr) {
					TSharedPtr<FJsonObject> Key = KeyPtr->AsObject();
					NewRichCurve.AddKey(Key->GetNumberField(TEXT("Time")), Key->GetNumberField(TEXT("Value")));
					FRichCurveKey RichKey = NewRichCurve.Keys.Last();

					RichKey.InterpMode =
						static_cast<ERichCurveInterpMode>(
							StaticEnum<ERichCurveInterpMode>()->GetValueByNameString(Key->GetStringField(TEXT("InterpMode")))
						);
					RichKey.TangentMode =
						static_cast<ERichCurveTangentMode>(
							StaticEnum<ERichCurveTangentMode>()->GetValueByNameString(Key->GetStringField(TEXT("TangentMode")))
						);
					RichKey.TangentWeightMode =
						static_cast<ERichCurveTangentWeightMode>(
							StaticEnum<ERichCurveTangentWeightMode>()->GetValueByNameString(Key->GetStringField(TEXT("TangentWeightMode")))
						);

					RichKey.ArriveTangent = Key->GetNumberField(TEXT("ArriveTangent"));
					RichKey.ArriveTangentWeight = Key->GetNumberField(TEXT("ArriveTangentWeight"));
					RichKey.LeaveTangent = Key->GetNumberField(TEXT("LeaveTangent"));
					RichKey.LeaveTangentWeight = Key->GetNumberField(TEXT("LeaveTangentWeight"));
				}
		} else {
			FSimpleCurve& NewSimpleCurve = CurveTable->AddSimpleCurve(FName(*Pair.Key)); {
				RealCurve = static_cast<FRealCurve>(NewSimpleCurve);
			}

			/* Method of Interpolation */
			NewSimpleCurve.InterpMode =
				static_cast<ERichCurveInterpMode>(
					StaticEnum<ERichCurveInterpMode>()->GetValueByNameString(CurveData->GetStringField(TEXT("InterpMode")))
				);

			if (const TArray<TSharedPtr<FJsonValue>>* KeysPtr; CurveData->TryGetArrayField(TEXT("Keys"), KeysPtr)) {
				for (const TSharedPtr<FJsonValue> KeyPtr : *KeysPtr) {
					TSharedPtr<FJsonObject> Key = KeyPtr->AsObject();
					
					NewSimpleCurve.AddKey(Key->GetNumberField(TEXT("Time")), Key->GetNumberField(TEXT("Value")));
				}
			}
		}

		/* Inherited data from FRealCurve */
		RealCurve.SetDefaultValue(CurveData->GetNumberField(TEXT("DefaultValue")));
		RealCurve.PreInfinityExtrap = 
			static_cast<ERichCurveExtrapolation>(
				StaticEnum<ERichCurveExtrapolation>()->GetValueByNameString(CurveData->GetStringField(TEXT("PreInfinityExtrap")))
			);
		RealCurve.PostInfinityExtrap =
			static_cast<ERichCurveExtrapolation>(
				StaticEnum<ERichCurveExtrapolation>()->GetValueByNameString(CurveData->GetStringField(TEXT("PostInfinityExtrap")))
			);

		/* Update Curve Table */
		CurveTable->OnCurveTableChanged().Broadcast();
		CurveTable->Modify(true);
	}

	/* Handle edit changes, and add it to the content browser */
	return OnAssetCreation(CurveTable);
}