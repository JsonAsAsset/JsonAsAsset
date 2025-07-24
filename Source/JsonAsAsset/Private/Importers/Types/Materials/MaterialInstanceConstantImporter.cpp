/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/Materials/MaterialInstanceConstantImporter.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Dom/JsonObject.h"
#include "RHIDefinitions.h"
#include "MaterialShared.h"

bool IMaterialInstanceConstantImporter::Import() {
	UMaterialInstanceConstant* MaterialInstanceConstant = NewObject<UMaterialInstanceConstant>(Package, UMaterialInstanceConstant::StaticClass(), *AssetName, RF_Public | RF_Standalone);

	GetObjectSerializer()->DeserializeObjectProperties(RemovePropertiesShared(AssetData,
	{
		"CachedReferencedTextures"
	}), MaterialInstanceConstant);

	TArray<TSharedPtr<FJsonValue>> StaticSwitchParametersObjects;
	TArray<TSharedPtr<FJsonValue>> StaticComponentMaskParametersObjects;
	
	/* Optional Editor Data [contains static switch parameters] ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	const TSharedPtr<FJsonObject> EditorOnlyData = GetExport("MaterialInstanceEditorOnlyData", AllJsonObjects, true);

	if (EditorOnlyData.IsValid()) {
		if (EditorOnlyData->HasField(TEXT("StaticParameters"))) {
			ReadStaticParameters(EditorOnlyData->GetObjectField(TEXT("StaticParameters")), StaticSwitchParametersObjects, StaticComponentMaskParametersObjects);
		}
	}

	/* Read from potential properties inside of asset data */
	if (AssetData->HasField(TEXT("StaticParametersRuntime"))) {
		ReadStaticParameters(AssetData->GetObjectField(TEXT("StaticParametersRuntime")), StaticSwitchParametersObjects, StaticComponentMaskParametersObjects);
	}
	if (AssetData->HasField(TEXT("StaticParameters"))) {
		ReadStaticParameters(AssetData->GetObjectField(TEXT("StaticParameters")), StaticSwitchParametersObjects, StaticComponentMaskParametersObjects);
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
		for (TSharedPtr<FJsonValue> Parameter : StaticParameters->GetArrayField(TEXT("StaticSwitchParameters"))) {
			StaticSwitchParameters.Add(TSharedPtr<FJsonValue>(Parameter));
		}
	}

	if (StaticParameters->HasField(TEXT("StaticComponentMaskParameters"))) {
		for (TSharedPtr<FJsonValue> Parameter : StaticParameters->GetArrayField(TEXT("StaticComponentMaskParameters"))) {
			StaticComponentMaskParameters.Add(TSharedPtr<FJsonValue>(Parameter));
		}
	}
}
