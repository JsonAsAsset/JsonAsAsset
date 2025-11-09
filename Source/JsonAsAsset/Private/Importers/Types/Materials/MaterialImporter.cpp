/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/Materials/MaterialImporter.h"

/* Include Material.h (depends on UE Version) */
#if (ENGINE_UE5 && ENGINE_MINOR_VERSION < 3) || ENGINE_UE4
#include "Materials/Material.h"
#else
#include "MaterialDomain.h"
#endif

#include "Factories/MaterialFactoryNew.h"
#include "Settings/JsonAsAssetSettings.h"

bool IMaterialImporter::Import() {
	/* Create Material Factory (factory automatically creates the Material) */
	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
	UMaterial* Material = Cast<UMaterial>(MaterialFactory->FactoryCreateNew(UMaterial::StaticClass(), OutermostPkg, *AssetName, RF_Standalone | RF_Public, nullptr, GWarn));

	/* Clear any default expressions the engine adds */
#if ENGINE_UE5
	Material->GetExpressionCollection().Empty();
#else
	Material->Expressions.Empty();
#endif

	/* Define material data from the JSON */
	FUObjectExportContainer ExpressionContainer;
	TSharedPtr<FJsonObject> Props = FindMaterialData(Material, JsonObject->GetStringField(TEXT("Type")), Material->GetName(), ExpressionContainer);

	/* Map out each expression for easier access */
	ConstructExpressions(ExpressionContainer);
	
	/* If Missing Material Data */
	if (ExpressionContainer.Num() == 0) {
		SpawnMaterialDataMissingNotification();

		return false;
	}

	/* Iterate through all the expressions, and set properties */
	PropagateExpressions(ExpressionContainer);

	const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();

#if ENGINE_UE5
	UMaterialEditorOnlyData* EditorOnlyData = Material->GetEditorOnlyData();
#else
	UMaterial* EditorOnlyData = Material;
#endif
	
	if (!Settings->AssetSettings.MaterialImportSettings.bSkipResultNodeConnection) {
		TArray<FString> IgnoredProperties = TArray<FString> {
			"ParameterGroupData",
			"ExpressionCollection",
			"CustomizedUVs"
		};

		const TSharedPtr<FJsonObject> RawConnectionData = TSharedPtr<FJsonObject>(Props);
		for (FString Property : IgnoredProperties) {
			if (RawConnectionData->HasField(Property))
			{
				RawConnectionData->RemoveField(Property);
			}
		}
		
		/* Connect all pins using deserializer */
		GetObjectSerializer()->DeserializeObjectProperties(RawConnectionData, EditorOnlyData);

		/* CustomizedUVs defined here */
		const TArray<TSharedPtr<FJsonValue>>* InputsPtr;
		
		if (Props->TryGetArrayField(TEXT("CustomizedUVs"), InputsPtr)) {
			int i = 0;
			for (const TSharedPtr<FJsonValue> InputValue : *InputsPtr) {
				FJsonObject* InputObject = InputValue->AsObject().Get();
				FName InputExpressionName = GetExpressionName(InputObject);

				if (ExpressionContainer.Contains(InputExpressionName)) {
					FExpressionInput Input = PopulateExpressionInput(InputObject, ExpressionContainer.Find<UMaterialExpression>(InputExpressionName));
					EditorOnlyData->CustomizedUVs[i] = *reinterpret_cast<FVector2MaterialInput*>(&Input);
				}
				i++;
			}
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* StringParameterGroupData;
	if (Props->TryGetArrayField(TEXT("ParameterGroupData"), StringParameterGroupData)) {
		TArray<FParameterGroupData> ParameterGroupData;

		for (const TSharedPtr<FJsonValue> ParameterGroupDataObject : *StringParameterGroupData) {
			if (ParameterGroupDataObject->IsNull()) continue;
			FParameterGroupData GroupData;

			FString GroupName;
			if (ParameterGroupDataObject->AsObject()->TryGetStringField(TEXT("GroupName"), GroupName)) GroupData.GroupName = GroupName;
			int GroupSortPriority;
			if (ParameterGroupDataObject->AsObject()->TryGetNumberField(TEXT("GroupSortPriority"), GroupSortPriority)) GroupData.GroupSortPriority = GroupSortPriority;

			ParameterGroupData.Add(GroupData);
		}

		EditorOnlyData->ParameterGroupData = ParameterGroupData;
	}

	/* Handle edit changes, and add it to the content browser */
	if (!OnAssetCreation(Material)) return false;

	const TSharedPtr<FJsonObject>* ShadingModelsPtr;
	
	if (AssetData->TryGetObjectField(TEXT("ShadingModels"), ShadingModelsPtr)) {
		int ShadingModelField;
		
		if (ShadingModelsPtr->Get()->TryGetNumberField(TEXT("ShadingModelField"), ShadingModelField)) {
#if ENGINE_UE5
			Material->GetShadingModels().SetShadingModelField(ShadingModelField);
#else
			/* Not to sure what to do in UE4, no function exists to override it. */
#endif
		}
	}

	/* Deserialize any properties */
	GetObjectSerializer()->DeserializeObjectProperties(AssetData, Material);

	Material->UpdateCachedExpressionData();
	
	FMaterialUpdateContext MaterialUpdateContext;
	MaterialUpdateContext.AddMaterial(Material);
	
	Material->ForceRecompileForRendering();

	Material->PostEditChange();
	Material->MarkPackageDirty();
	Material->PreEditChange(nullptr);

	Save();

	return true;
}