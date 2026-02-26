/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Importers/Types/Curves/CurveLinearColorAtlasImporter.h"
#include "Curves/CurveLinearColorAtlas.h"
#include "Curves/CurveLinearColor.h"

UObject* ICurveLinearColorAtlasImporter::CreateAsset(UObject* CreatedAsset) {
	return IImporter::CreateAsset(NewObject<UCurveLinearColorAtlas>(GetPackage(), UCurveLinearColorAtlas::StaticClass(), *GetAssetName(), RF_Public | RF_Standalone));
}

bool ICurveLinearColorAtlasImporter::Import() {
	float Width = 256;
	float Height = 256;

	UCurveLinearColorAtlas* Object = Create<UCurveLinearColorAtlas>();
#if !UE5_6_BEYOND
	Object->Source.Init(Width, Height, 1, 1, TSF_RGBA16F);
	const int32 TextureDataSize = Object->Source.CalcMipSize(0);
	Object->SrcData.AddUninitialized(TextureDataSize);
	uint32* TextureData = reinterpret_cast<uint32*>(Object->Source.LockMip(0));
	FFloat16Color InitColor(FLinearColor::White);
	for (uint32 y = 0; y < Object->TextureSize; y++) {
		for (uint32 x = 0; x < Object->TextureSize; x++) {
			Object->SrcData[x * Object->TextureSize + y] = InitColor;
		}
	}
	FMemory::Memcpy(TextureData, Object->SrcData.GetData(), TextureDataSize);
	Object->Source.UnlockMip(0);

	Object->UpdateResource();

	bool bHasAnyDirtyTextures = false;
	if (GetAssetData()->TryGetBoolField(TEXT("bHasAnyDirtyTextures"), bHasAnyDirtyTextures)) {
		Object->bHasAnyDirtyTextures = bHasAnyDirtyTextures;
	}

	bool bIsDirty = false;
	if (GetAssetData()->TryGetBoolField(TEXT("bIsDirty"), bIsDirty)) {
		Object->bIsDirty = bIsDirty;
	}

	bool bShowDebugColorsForNullGradients = false;
	if (GetAssetData()->TryGetBoolField(TEXT("bShowDebugColorsForNullGradients"), bShowDebugColorsForNullGradients)) {
		Object->bShowDebugColorsForNullGradients = bShowDebugColorsForNullGradients;
	}

	bool bSquareResolution = false;
#else
	Object->TextureSize = FMath::Max(Width,2);
	Object->UpdateTextures();
#endif
	
#if !UE4_27_ONLY_BELOW && !UE5_6_BEYOND
	if (GetAssetData()->TryGetBoolField(TEXT("bSquareResolution"), bSquareResolution)) {
		Object->bSquareResolution = bSquareResolution;
	}
#endif

#if ENGINE_UE5
	float TextureSize = 0.0f;
	float TextureHeight = 0.0f;
#else
	double TextureSize = 0.0f;
	double TextureHeight = 0.0f;
#endif
	
	if (GetAssetData()->TryGetNumberField(TEXT("TextureSize"), TextureSize)) {
		Object->TextureSize = TextureSize;
	}

#if !UE4_27_ONLY_BELOW && !UE5_6_BEYOND
	if (GetAssetData()->TryGetNumberField(TEXT("TextureHeight"), TextureHeight)) {
		Object->TextureHeight = TextureHeight;
	}
#endif

	FProperty* TextureSizeProperty = FindFProperty<FProperty>(Object->GetClass(), "TextureSize");
	FPropertyChangedEvent TextureSizePropertyPropertyChangedEvent(TextureSizeProperty, EPropertyChangeType::ValueSet);
	Object->PostEditChangeProperty(TextureSizePropertyPropertyChangedEvent);

	const TArray<TSharedPtr<FJsonValue>> GradientCurves = GetAssetData()->GetArrayField(TEXT("GradientCurves"));

	TWeakObjectPtr<UCurveLinearColorAtlas> WeakObject = Object;
	TSharedRef<TArray<UCurveLinearColor*>> CurvesToAdd = MakeShared<TArray<UCurveLinearColor*>, ESPMode::ThreadSafe>();
	TSharedRef<FThreadSafeCounter> PendingLoads = MakeShared<FThreadSafeCounter, ESPMode::ThreadSafe>();

	PendingLoads->Set(GradientCurves.Num());

	for (const TSharedPtr<FJsonValue>& ArrayElement : GradientCurves) {
		const TSharedPtr<FJsonObject> ObjectPtr = ArrayElement->AsObject();

		LoadExport<UObject>(&ObjectPtr, [WeakObject, CurvesToAdd, PendingLoads] (const TObjectPtr<UObject> LoadedObject) {
			if (!WeakObject.IsValid()) {
				return;
			}

			if (UCurveLinearColor* Curve = Cast<UCurveLinearColor>(LoadedObject.Get())) {
				CurvesToAdd->Add(Curve);
			}

			if (PendingLoads->Decrement() == 0) {
				AsyncTask(ENamedThreads::GameThread,
				[WeakObject, CurvesToAdd]() {
					if (!WeakObject.IsValid()) {
						return;
					}

					UCurveLinearColorAtlas* Object = WeakObject.Get();

					Object->GradientCurves.Append(*CurvesToAdd);

					FProperty* GradientCurvesProperty =
						FindFProperty<FProperty>(
							Object->GetClass(),
							TEXT("GradientCurves"));

					FPropertyChangedEvent PropertyChangedEvent(
						GradientCurvesProperty,
						EPropertyChangeType::ArrayAdd);

					Object->PostEditChangeProperty(PropertyChangedEvent);
				});
			}
		});
	}

	/* Handle edit changes, and add it to the content browser */
	return OnAssetCreation(Object);
}