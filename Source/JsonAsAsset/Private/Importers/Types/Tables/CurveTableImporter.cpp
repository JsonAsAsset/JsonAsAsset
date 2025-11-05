/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/Tables/CurveTableImporter.h"
#include "Dom/JsonObject.h"

class UCurveTableAccessor final : public UCurveTable {
public:
	void ChangeTableMode(const ECurveTableMode Mode) {
		CurveTableMode = Mode;
	}
};

bool ICurveTableImporter::Import() {
	const TSharedPtr<FJsonObject> RowData = AssetData->GetObjectField(TEXT("Rows"));
	UCurveTable* CurveTable = NewObject<UCurveTable>(Package, UCurveTable::StaticClass(), *AssetName, RF_Public | RF_Standalone);

	/* Used to determine curve type */
	ECurveTableMode CurveTableMode = ECurveTableMode::RichCurves; {
		if (FString CurveMode; AssetData->TryGetStringField(TEXT("CurveTableMode"), CurveMode))
			CurveTableMode = static_cast<ECurveTableMode>(StaticEnum<ECurveTableMode>()->GetValueByNameString(CurveMode));

		UCurveTableAccessor* CurveTableAccessor = Cast<UCurveTableAccessor>(CurveTable);
		CurveTableAccessor->ChangeTableMode(CurveTableMode);
	}

	/* Loop throughout row data, and deserialize */
	for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : RowData->Values) {
		const TSharedPtr<FJsonObject> CurveData = Pair.Value->AsObject();

		if (CurveTableMode == ECurveTableMode::RichCurves) {
			FRichCurve& Curve = CurveTable->AddRichCurve(FName(*Pair.Key));

			GetPropertySerializer()->DeserializeStruct(Curve.StaticStruct(), CurveData.ToSharedRef(), &Curve);
		} else {
			FSimpleCurve& Curve = CurveTable->AddSimpleCurve(FName(*Pair.Key));
			
			GetPropertySerializer()->DeserializeStruct(Curve.StaticStruct(), CurveData.ToSharedRef(), &Curve);
		}

		/* Update Curve Table */
		CurveTable->OnCurveTableChanged().Broadcast();
		CurveTable->Modify(true);
	}

	/* Handle edit changes, and add it to the content browser */
	return OnAssetCreation(CurveTable);
}