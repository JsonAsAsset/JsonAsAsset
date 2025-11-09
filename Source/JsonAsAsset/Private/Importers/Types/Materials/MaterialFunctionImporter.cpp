/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/Materials/MaterialFunctionImporter.h"
#include "Factories/MaterialFunctionFactoryNew.h"

bool IMaterialFunctionImporter::Import() {
	/* Create Material Function Factory (factory automatically creates the Material Function) */
	UMaterialFunctionFactoryNew* MaterialFunctionFactory = NewObject<UMaterialFunctionFactoryNew>();
	UMaterialFunction* MaterialFunction = Cast<UMaterialFunction>(MaterialFunctionFactory->FactoryCreateNew(UMaterialFunction::StaticClass(), OutermostPkg, *AssetName, RF_Standalone | RF_Public, nullptr, GWarn));

	/* Empty all expressions, we create them */
#if ENGINE_UE5
	MaterialFunction->GetExpressionCollection().Empty();
#else
	MaterialFunction->FunctionExpressions.Empty();
#endif

	/* Handle edit changes, and add it to the content browser */
	if (!OnAssetCreation(MaterialFunction)) return false;

	/* Define editor only data from the JSON */
	FUObjectExportContainer ExpressionContainer;

	const TSharedPtr<FJsonObject> Props = FindMaterialData(MaterialFunction, JsonObject->GetStringField(TEXT("Type")), MaterialFunction->GetName(), ExpressionContainer);

	/* Map out each expression for easier access */
	ConstructExpressions(ExpressionContainer);

	/* If Missing Material Data */
	if (ExpressionContainer.Num() == 0) {
		SpawnMaterialDataMissingNotification();

		return false;
	}

	/* Iterate through all the expressions, and set properties */
	PropagateExpressions(ExpressionContainer);

	/* Deserialize any properties */
	GetObjectSerializer()->DeserializeObjectProperties(AssetData, MaterialFunction);
	
	MaterialFunction->PreEditChange(nullptr);
	MaterialFunction->PostEditChange();

	Save();
	
	return true;
}
