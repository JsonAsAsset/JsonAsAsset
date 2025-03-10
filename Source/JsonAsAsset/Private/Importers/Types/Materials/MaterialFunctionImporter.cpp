/* Copyright JAA Contributors 2024-2025 */

#include "Importers/Types/Materials/MaterialFunctionImporter.h"
#include "Factories/MaterialFunctionFactoryNew.h"

bool IMaterialFunctionImporter::Import() {
	/* Create Material Function Factory (factory automatically creates the Material Function) */
	UMaterialFunctionFactoryNew* MaterialFunctionFactory = NewObject<UMaterialFunctionFactoryNew>();
	UMaterialFunction* MaterialFunction = Cast<UMaterialFunction>(MaterialFunctionFactory->FactoryCreateNew(UMaterialFunction::StaticClass(), OutermostPkg, *FileName, RF_Standalone | RF_Public, nullptr, GWarn));

	/* Empty all expressions, we create them */
#if ENGINE_MAJOR_VERSION >= 5
	MaterialFunction->GetExpressionCollection().Empty();
#else
	MaterialFunction->FunctionExpressions.Empty();
#endif

	/* Handle edit changes, and add it to the content browser */
	if (!HandleAssetCreation(MaterialFunction)) return false;

	/* Define editor only data from the JSON */
	FMaterialExpressionNodeExportContainer ExpressionContainer;

	const TSharedPtr<FJsonObject> EdProps = FindEditorOnlyData(JsonObject->GetStringField(TEXT("Type")), MaterialFunction->GetName(), ExpressionContainer);

	/* Map out each expression for easier access */
	ConstructExpressions(MaterialFunction, ExpressionContainer);

	/* If Missing Material Data */
	if (ExpressionContainer.Num() == 0) {
		SpawnMaterialDataMissingNotification();

		return false;
	}

	/* Iterate through all the expressions, and set properties */
	PropagateExpressions(MaterialFunction, ExpressionContainer);

	/* Create comments */
	CreateExtraNodeInformation(MaterialFunction);

	/* Deserialize any properties */
	GetObjectSerializer()->DeserializeObjectProperties(AssetData, MaterialFunction);
	
	MaterialFunction->PreEditChange(nullptr);
	MaterialFunction->PostEditChange();

	SavePackage();
	
	return true;
}
