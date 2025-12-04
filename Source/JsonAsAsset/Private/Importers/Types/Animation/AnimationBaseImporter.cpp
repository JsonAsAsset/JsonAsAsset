/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/Animation/AnimationBaseImporter.h"

#include "Animation/AnimMontage.h"
#include "Dom/JsonObject.h"
#include "Animation/AnimSequence.h"
#include "Utilities/JsonUtilities.h"

#if ENGINE_UE5
#include "Animation/AnimData/IAnimationDataController.h"
#if ENGINE_MINOR_VERSION >= 4
#include "Animation/AnimData/IAnimationDataModel.h"
#endif
#include "AnimDataController.h"
#endif

bool IAnimationBaseImporter::Import() {
	const FString JsonName = GetAssetExport()->GetStringField(TEXT("Name"));

	TArray<TSharedPtr<FJsonValue>> FloatCurves;
	TArray<TSharedPtr<FJsonValue>> Notifies;

	UAnimSequenceBase* AnimSequenceBase = GetSelectedAsset<UAnimSequenceBase>(true, JsonName);

	if (!AnimSequenceBase && GetAssetClass()->IsChildOf<UAnimMontage>()) {
		AnimSequenceBase = NewObject<UAnimMontage>(GetPackage(), GetAssetClass(), *JsonName, RF_Public | RF_Standalone);
	}

	if (!AnimSequenceBase) {
		UE_LOG(LogJsonAsAsset, Error, TEXT("Could not get valid AnimSequenceBase"));
		const FText DialogText = FText::Format(
			FText::FromString(TEXT("Importing an asset of type '{0}' requires a base asset selected to modify. Select one in your content browser.")),
			FText::FromString("AnimationBaseImporter")
		);

		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		
		return false;
	}

	UAnimSequence* CastedAnimSequence = Cast<UAnimSequence>(AnimSequenceBase);

	if (CastedAnimSequence) {
		CastedAnimSequence->AuthoredSyncMarkers.Empty();
		CastedAnimSequence->Notifies.Empty();
	}

	GetObjectSerializer()->SetExportForDeserialization(GetAssetExport(), AnimSequenceBase);
	GetObjectSerializer()->Parent = AnimSequenceBase;

	GetObjectSerializer()->DeserializeExports(JsonObjects);

	/* Deserialize properties */
	GetObjectSerializer()->DeserializeObjectProperties(RemovePropertiesShared(GetAssetData(), {
		"NumFrames",
		"TrackToSkeletonMapTable",
		"SequenceLength",
		"Skeleton",
		"SkeletonGuid",
		"CompressedTrackToSkeletonMapTable",
		"CompressedDataStructure",
		"CompressedRawDataSize",
		"RawCurveData"
	}), AnimSequenceBase);

	USkeleton* Skeleton = AnimSequenceBase->GetSkeleton();
	ensure(Skeleton);
	if (!Skeleton) {
		UE_LOG(LogJsonAsAsset, Error, TEXT("Could not get valid Skeleton"));
		return false;
	}
	
	/* In Unreal Engine 5, a new data model has been added to edit animation curves */
	/* Unreal Engine 5.2 changed handling getting a data model */
#if UE5_2_BEYOND
	IAnimationDataController& Controller = AnimSequenceBase->GetController();
#if ENGINE_MINOR_VERSION >= 3
	IAnimationDataModel* DataModel = AnimSequenceBase->GetDataModel();
#endif
#endif

	/* Some CUEParse versions have different named objects for curves ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	const TSharedPtr<FJsonObject>* RawCurveData;
	
	if (GetAssetData()->TryGetObjectField(TEXT("RawCurveData"), RawCurveData))
		FloatCurves = GetAssetData()->GetObjectField(TEXT("RawCurveData"))->GetArrayField(TEXT("FloatCurves"));
	
	if (GetAssetExport()->TryGetObjectField(TEXT("CompressedCurveData"), RawCurveData))
		FloatCurves = GetAssetExport()->GetObjectField(TEXT("CompressedCurveData"))->GetArrayField(TEXT("FloatCurves"));

	/* Import the curves */
	for (const TSharedPtr<FJsonValue> FloatCurveObject : FloatCurves) {
		/* Curve Display Name */
		FString DisplayName = "";
		if (FloatCurveObject->AsObject()->HasField(TEXT("Name"))) {
			DisplayName = FloatCurveObject->AsObject()->GetObjectField(TEXT("Name"))->GetStringField(TEXT("DisplayName"));
		} else {
			DisplayName = FloatCurveObject->AsObject()->GetStringField(TEXT("CurveName"));
		}

		/* Used to define if a curve is a curve is metadata or not. */
		int CurveTypeFlags = FloatCurveObject->AsObject()->GetIntegerField(TEXT("CurveTypeFlags"));

		/* Adding the track name to skeletons differ between Unreal Engine 4 and 5 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#if ENGINE_UE4
		FSmartName NewTrackName;

		Skeleton->AddSmartNameAndModify(USkeleton::AnimCurveMappingName, FName(*DisplayName), NewTrackName);
		ensureAlways(Skeleton->GetSmartNameByUID(USkeleton::AnimCurveMappingName, NewTrackName.UID, NewTrackName));
#endif
		
#if ENGINE_UE5
#if ENGINE_MINOR_VERSION <= 3
		FSmartName NewTrackName;

		Skeleton->AddSmartNameAndModify(USkeleton::AnimCurveMappingName, FName(*DisplayName), NewTrackName);
		
		ensureAlways(Skeleton->GetSmartNameByUID(USkeleton::AnimCurveMappingName, NewTrackName.UID, NewTrackName));
		FAnimationCurveIdentifier CurveId = FAnimationCurveIdentifier(NewTrackName, ERawCurveTrackTypes::RCT_Float);
#endif
		/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
		
#if ENGINE_MINOR_VERSION >= 4
		Controller.OpenBracket(FText::FromString("Curve Import"));

		/* Create Curve Identifier */
		FName CurveName = FName(*DisplayName);
		FAnimationCurveIdentifier CurveId(CurveName, ERawCurveTrackTypes::RCT_Float);

		/* Add curve metadata to skeleton */
		Skeleton->AddCurveMetaData(CurveName);
		/* Add or update the curve */
		const FFloatCurve* ExistingCurve = DataModel->FindFloatCurve(CurveId);
		if (ExistingCurve != nullptr)
		{
			Controller.RemoveCurve(CurveId);
		}
		Controller.AddCurve(CurveId, CurveTypeFlags);
#endif
		/* For Unreal Engine 5.3 and above, the smart name's display name is required */
#if ENGINE_MINOR_VERSION == 3 && ENGINE_PATCH_VERSION < 2
		Controller->AddCurve(CurveId, CurveTypeFlags);
#elif (ENGINE_MINOR_VERSION == 3 && ENGINE_PATCH_VERSION == 2)
		Controller.AddCurve(CurveId, CurveTypeFlags);
#endif
		/* For Unreal Engine 5.2 and below, just the smart name is required */
#if ENGINE_MINOR_VERSION < 3
		AnimSequenceBase->Modify(true);

		IAnimationDataController& LocalOneController = AnimSequenceBase->GetController();
		LocalOneController.AddCurve(FAnimationCurveIdentifier(NewTrackName, ERawCurveTrackTypes::RCT_Float), CurveTypeFlags);

		TArray<FLinearColor> RandomizedColorArray = {
			FLinearColor(.904, .323, .539),
			FLinearColor(.552, .737, .328),
			FLinearColor(.947, .418, .219),
			FLinearColor(.156, .624, .921),
			FLinearColor(.921, .314, .337),
			FLinearColor(.361, .651, .332),
			FLinearColor(.982, .565, .254),
			FLinearColor(.246, .223, .514),
			FLinearColor(.208, .386, .687),
			FLinearColor(.223, .590, .337),
			FLinearColor(.230, .291, .591)
		}; { 
			auto Index = rand() % RandomizedColorArray.Num();

			if (RandomizedColorArray.IsValidIndex(Index)) {
				LocalOneController.SetCurveColor(FAnimationCurveIdentifier(NewTrackName, ERawCurveTrackTypes::RCT_Float), RandomizedColorArray[Index]);
			}
		}

		AnimSequenceBase->PostEditChange();
#endif
#endif

		/* Keys of the track */
		TArray<TSharedPtr<FJsonValue>> Keys = FloatCurveObject->AsObject()->GetObjectField(TEXT("FloatCurve"))->GetArrayField(TEXT("Keys"));

		for (const TSharedPtr<FJsonValue> JsonKey : Keys) {
			TSharedPtr<FJsonObject> Key = JsonKey->AsObject();

			FRichCurveKey RichKey = ObjectToRichCurveKey(Key);

			/*
			 * Unreal Engine 5 and Unreal Engine 4
			 * have different ways of adding curves
			 *
			 * Unreal Engine 4: Simply adding curves to RawCurveData
			 * Unreal Engine 5: Using a AnimDataController to handle adding curves
			*/
#if UE5_2_BEYOND
			Controller.SetCurveKey(CurveId, RichKey);
#endif
#if ENGINE_UE4
			FRawCurveTracks& Tracks = AnimSequenceBase->RawCurveData;
#endif
#if ENGINE_UE4
			Tracks.AddFloatCurveKey(NewTrackName, CurveTypeFlags, RichKey.Time, RichKey.Value);

			for (FFloatCurve& Track : Tracks.FloatCurves) {
				if (Track.Name == NewTrackName) {
					const int32 LastIndex = Track.FloatCurve.Keys.Num() - 1;
					Track.FloatCurve.Keys[LastIndex].ArriveTangent = RichKey.ArriveTangent;
					Track.FloatCurve.Keys[LastIndex].LeaveTangent = RichKey.LeaveTangent;
					Track.FloatCurve.Keys[LastIndex].InterpMode = RichKey.InterpMode;
				}
			}
#endif
#if UE5_1_BELOW
#endif
		}
#if UE5_2_BEYOND
		Controller.CloseBracket();
#endif
	}

#if UE5_2_BEYOND
	if (ITargetPlatform* RunningPlatform = GetTargetPlatformManagerRef().GetRunningTargetPlatform()) {
#if UE5_6_BEYOND
		CastedAnimSequence->CacheDerivedDataForPlatform(RunningPlatform);
#else
		CastedAnimSequence->CacheDerivedData(RunningPlatform);
#endif
	}
#else
	if (CastedAnimSequence) {
		CastedAnimSequence->RequestSyncAnimRecompression();
	}
#endif

#if ENGINE_UE4
	AnimSequenceBase->MarkRawDataAsModified();
#endif
	AnimSequenceBase->Modify();
	AnimSequenceBase->PostEditChange();

	return OnAssetCreation(AnimSequenceBase);
}
