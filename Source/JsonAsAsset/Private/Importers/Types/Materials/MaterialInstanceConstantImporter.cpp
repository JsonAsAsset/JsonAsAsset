/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/Materials/MaterialInstanceConstantImporter.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Dom/JsonObject.h"
#include "RHIDefinitions.h"
#include "MaterialShared.h"

UObject* IMaterialInstanceConstantImporter::CreateAsset(UObject* CreatedAsset) {
	return IImporter::CreateAsset(NewObject<UMaterialInstanceConstant>(GetPackage(), UMaterialInstanceConstant::StaticClass(), *GetAssetName(), RF_Public | RF_Standalone));
}

bool IMaterialInstanceConstantImporter::Import() {
	UMaterialInstanceConstant* MaterialInstanceConstant = Create<UMaterialInstanceConstant>();

	/* Specific fix for 4.16 engines */
	const TArray<FString> ParameterFields = {
		TEXT("ScalarParameterValues"),
		TEXT("TextureParameterValues"),
		TEXT("VectorParameterValues")
	};

	for (const FString& FieldName : ParameterFields) {
		if (GetAssetData()->HasField(FieldName)) {
			TArray<TSharedPtr<FJsonValue>> Params = GetAssetData()->GetArrayField(FieldName);
			ConvertParameterNamesToInfos(Params);
			GetAssetData()->SetArrayField(FieldName, Params);
		}
	}
	
	GetObjectSerializer()->DeserializeObjectProperties(RemovePropertiesShared(GetAssetData(),
	{
		"CachedReferencedTextures"
	}), MaterialInstanceConstant);

	TArray<TSharedPtr<FJsonValue>> StaticSwitchParametersObjects;
	TArray<TSharedPtr<FJsonValue>> StaticComponentMaskParametersObjects;
	
	/* Optional Editor Data [contains static switch parameters] ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	const TSharedPtr<FJsonObject> EditorOnlyData = GetExport("MaterialInstanceEditorOnlyData", JsonObjects, true);

	if (EditorOnlyData.IsValid()) {
		if (EditorOnlyData->HasField(TEXT("StaticParameters"))) {
			ReadStaticParameters(EditorOnlyData->GetObjectField(TEXT("StaticParameters")), StaticSwitchParametersObjects, StaticComponentMaskParametersObjects);
		}
	}

	/* Read from potential properties inside of asset data */
	if (GetAssetData()->HasField(TEXT("StaticParametersRuntime"))) {
		ReadStaticParameters(GetAssetData()->GetObjectField(TEXT("StaticParametersRuntime")), StaticSwitchParametersObjects, StaticComponentMaskParametersObjects);
	}
	if (GetAssetData()->HasField(TEXT("StaticParameters"))) {
		ReadStaticParameters(GetAssetData()->GetObjectField(TEXT("StaticParameters")), StaticSwitchParametersObjects, StaticComponentMaskParametersObjects);
	}

	/* ~~~~~~~~~ STATIC PARAMETERS ~~~~~~~~~~~ */
#if UE5_2_BEYOND || UE4_27_BELOW
	FStaticParameterSet NewStaticParameterSet; /* Unreal Engine 5.2/4.26 and beyond have a different method */
#endif

	TArray<FStaticSwitchParameter> StaticSwitchParameters;
	for (const TSharedPtr<FJsonValue> StaticParameter_Value : StaticSwitchParametersObjects) {
		TSharedPtr<FJsonObject> ParameterObject = StaticParameter_Value->AsObject();
		TSharedPtr<FJsonObject> Local_MaterialParameterInfo = ParameterObject->GetObjectField(TEXT("ParameterInfo"));

		/* Create Material Parameter Info */
		FMaterialParameterInfo MaterialParameterParameterInfo = FMaterialParameterInfo(
			FName(Local_MaterialParameterInfo->GetStringField(TEXT("Name"))),
			static_cast<EMaterialParameterAssociation>(StaticEnum<EMaterialParameterAssociation>()->GetValueByNameString(Local_MaterialParameterInfo->GetStringField(TEXT("Association")))),
			Local_MaterialParameterInfo->GetIntegerField(TEXT("Index"))
		);

		/* Now, create the actual switch parameter */
		FStaticSwitchParameter Parameter = FStaticSwitchParameter(
			MaterialParameterParameterInfo,
			ParameterObject->GetBoolField(TEXT("Value")),
			ParameterObject->GetBoolField(TEXT("bOverride")),
			FGuid(ParameterObject->GetStringField(TEXT("ExpressionGUID")))
		);

		StaticSwitchParameters.Add(Parameter);
#if UE5_1_BELOW
		MaterialInstanceConstant->GetEditorOnlyData()->StaticParameters.StaticSwitchParameters.Add(Parameter);
#endif
		
#if UE5_2_BEYOND || UE4_27_BELOW
		/* Unreal Engine 5.2/4.26 and beyond have a different method */
		NewStaticParameterSet.StaticSwitchParameters.Add(Parameter); 
#endif
	}

	TArray<FStaticComponentMaskParameter> StaticSwitchMaskParameters;
	
	for (const TSharedPtr<FJsonValue> StaticParameter_Value : StaticComponentMaskParametersObjects) {
		TSharedPtr<FJsonObject> ParameterObject = StaticParameter_Value->AsObject();
		TSharedPtr<FJsonObject> Local_MaterialParameterInfo = ParameterObject->GetObjectField(TEXT("ParameterInfo"));

		/* Create Material Parameter Info */
		FMaterialParameterInfo MaterialParameterParameterInfo = FMaterialParameterInfo(
			FName(Local_MaterialParameterInfo->GetStringField(TEXT("Name"))),
			static_cast<EMaterialParameterAssociation>(StaticEnum<EMaterialParameterAssociation>()->GetValueByNameString(Local_MaterialParameterInfo->GetStringField(TEXT("Association")))),
			Local_MaterialParameterInfo->GetIntegerField(TEXT("Index"))
		);

		FStaticComponentMaskParameter Parameter = FStaticComponentMaskParameter(
			MaterialParameterParameterInfo,
			ParameterObject->GetBoolField(TEXT("R")),
			ParameterObject->GetBoolField(TEXT("G")),
			ParameterObject->GetBoolField(TEXT("B")),
			ParameterObject->GetBoolField(TEXT("A")),
			ParameterObject->GetBoolField(TEXT("bOverride")),
			FGuid(ParameterObject->GetStringField(TEXT("ExpressionGUID")))
		);

		StaticSwitchMaskParameters.Add(Parameter);
#if UE5_1_BELOW
		MaterialInstanceConstant->GetEditorOnlyData()->StaticParameters.StaticComponentMaskParameters.Add(Parameter);
#endif
		
#if UE5_2_BEYOND || UE4_27_BELOW
		NewStaticParameterSet.
		/* EditorOnly is needed on 5.2+ */
		#if UE5_2_BEYOND
			EditorOnly.
		#endif
		StaticComponentMaskParameters.Add(Parameter);
#endif
	}

#if UE5_2_BEYOND || UE4_27_BELOW
	FMaterialUpdateContext MaterialUpdateContext(FMaterialUpdateContext::EOptions::Default & ~FMaterialUpdateContext::EOptions::RecreateRenderStates);

	MaterialInstanceConstant->UpdateStaticPermutation(NewStaticParameterSet, &MaterialUpdateContext);
	MaterialInstanceConstant->InitStaticPermutation();
#endif

	return OnAssetCreation(MaterialInstanceConstant);
}

void IMaterialInstanceConstantImporter::ReadStaticParameters(const TSharedPtr<FJsonObject>& StaticParameters, TArray<TSharedPtr<FJsonValue>>& StaticSwitchParameters, TArray<TSharedPtr<FJsonValue>>& StaticComponentMaskParameters) {
	if (StaticParameters->HasField(TEXT("StaticSwitchParameters"))) {
		TArray<TSharedPtr<FJsonValue>> Params = StaticParameters->GetArrayField("StaticSwitchParameters");
		ConvertParameterNamesToInfos(Params);
		
		for (TSharedPtr<FJsonValue> Parameter : Params) {
			StaticSwitchParameters.Add(TSharedPtr<FJsonValue>(Parameter));
		}
	}

	if (StaticParameters->HasField(TEXT("StaticComponentMaskParameters"))) {
		TArray<TSharedPtr<FJsonValue>> Params = StaticParameters->GetArrayField("StaticComponentMaskParameters");
		ConvertParameterNamesToInfos(Params);
		
		for (TSharedPtr<FJsonValue> Parameter : Params) {
			StaticComponentMaskParameters.Add(TSharedPtr<FJsonValue>(Parameter));
		}
	}
}

void IMaterialInstanceConstantImporter::ConvertParameterNamesToInfos(TArray<TSharedPtr<FJsonValue>>& Input) {
	/* Convert ParameterName to be inside ParameterInfo */
	for (const TSharedPtr<FJsonValue>& Parameter : Input) {
		const TSharedPtr<FJsonObject>& ParameterObject = Parameter->AsObject();

		if (ParameterObject->HasField(TEXT("ParameterName"))) {
			TSharedPtr<FJsonObject> ParameterInfo = MakeShared<FJsonObject>();
			
			ParameterInfo->SetStringField(TEXT("Name"), ParameterObject->GetStringField(TEXT("ParameterName")));
			ParameterObject->SetObjectField("ParameterInfo", ParameterInfo);

			/* Cleanup */
			ParameterObject->RemoveField("ParameterName");
		}
	}
}