/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"

/*
 * Material Graph Handler
 * Handles everything needed to create a material graph from JSON.
*/
class IMaterialGraph : public IImporter {
public:
	IMaterialGraph(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& JsonObjects):
		IImporter(JsonObject, Package, JsonObjects) {
	}
	
protected:
	/* Find Material's Data, and creates a container of material nodes */
	TSharedPtr<FJsonObject> FindMaterialData(const FString& Type, FUObjectExportContainer& Container);

	/* Functions to Handle Expressions */
	static void SetExpressionParent(UObject* Parent, UMaterialExpression* Expression, const TSharedPtr<FJsonObject>& Json);
	static void AddExpressionToParent(UObject* Parent, UMaterialExpression* Expression);
	
	/* Makes each expression with their class */
	void ConstructExpressions(FUObjectExportContainer& Container);
	UMaterialExpression* CreateEmptyExpression(FUObjectExport& Export, FUObjectExportContainer& Container);

	/* Modifies Graph Nodes (copies over properties from FJsonObject) */
	void PropagateExpressions(FUObjectExportContainer& Container);
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

	/* Functions to Handle Node Connections ~~~~~~~~~~~~ */
	static FExpressionInput PopulateExpressionInput(const FJsonObject* JsonProperties, UMaterialExpression* Expression, const FString& Type = "Default");
	static FName GetExpressionName(const FJsonObject* JsonProperties, const FString& OverrideParameterName = "Expression");

public:
	UMaterialExpression* OnMissingNodeClass(FUObjectExport& Export, FUObjectExportContainer& Container);
	void SpawnMaterialDataMissingNotification() const;

#if ENGINE_UE4
	/*
	 * In Unreal Engine 4, to combat the absence of Sub-graphs, create a Material Function in place of it
	 * This holds a mapping to the name of the composite node it was created from, and the material
	 * function created in-place of it
	 */

	TMap<FName, UMaterialFunction*> SubgraphFunctions;
#endif
};
