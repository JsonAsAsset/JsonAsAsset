/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/Curves/CurveVectorImporter.h"
#include "Factories/CurveFactory.h"
#include "Curves/CurveVector.h"
#include "Utilities/JsonUtilities.h"

bool ICurveVectorImporter::Import() {
	/* Array of containers */
	TArray<TSharedPtr<FJsonValue>> FloatCurves = AssetData->GetArrayField(TEXT("FloatCurves"));

	UCurveVectorFactory* CurveVectorFactory = NewObject<UCurveVectorFactory>();
	UCurveVector* CurveVectorAsset = Cast<UCurveVector>(CurveVectorFactory->FactoryCreateNew(UCurveVector::StaticClass(), OutermostPackage, *AssetName, RF_Standalone | RF_Public, nullptr, GWarn));

	/* For each container, get keys */
	for (int i = 0; i < FloatCurves.Num(); i++) {
		TArray<TSharedPtr<FJsonValue>> Keys = FloatCurves[i]->AsObject()->GetArrayField(TEXT("Keys"));
		CurveVectorAsset->FloatCurves[i].Keys.Empty();

		/* Add keys to the array */
		for (int j = 0; j < Keys.Num(); j++) {
			CurveVectorAsset->FloatCurves[i].Keys.Add(ObjectToRichCurveKey(Keys[j]->AsObject()));
		}
	}

	/* Handle edit changes, and add it to the content browser */
	return OnAssetCreation(CurveVectorAsset);
}
