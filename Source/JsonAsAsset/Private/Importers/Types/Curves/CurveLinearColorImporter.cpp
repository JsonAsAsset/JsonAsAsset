/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/Curves/CurveLinearColorImporter.h"
#include "Curves/CurveLinearColor.h"
#include "Factories/CurveFactory.h"
#include "Utilities/JsonUtilities.h"

bool ICurveLinearColorImporter::Import() {
	TArray<TSharedPtr<FJsonValue>> FloatCurves = AssetData->GetArrayField(TEXT("FloatCurves"));

	UCurveLinearColorFactory* CurveFactory = NewObject<UCurveLinearColorFactory>();
	UCurveLinearColor* LinearCurveAsset = Cast<UCurveLinearColor>(CurveFactory->FactoryCreateNew(UCurveLinearColor::StaticClass(), OutermostPkg, *AssetName, RF_Standalone | RF_Public, nullptr, GWarn));

	/* For each container, get keys */
	for (int i = 0; i < FloatCurves.Num(); i++) {
		TArray<TSharedPtr<FJsonValue>> Keys = FloatCurves[i]->AsObject()->GetArrayField(TEXT("Keys"));
		LinearCurveAsset->FloatCurves[i].Keys.Empty();

		/* Add keys to the array */
		for (int j = 0; j < Keys.Num(); j++) {
			LinearCurveAsset->FloatCurves[i].Keys.Add(ObjectToRichCurveKey(Keys[j]->AsObject()));
		}
	}

	return OnAssetCreation(LinearCurveAsset);
}
