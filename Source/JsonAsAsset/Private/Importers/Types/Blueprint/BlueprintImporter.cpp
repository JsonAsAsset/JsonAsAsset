/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Importers/Types/Blueprint/BlueprintImporter.h"

#include "KismetCompilerModule.h"
#include "WidgetBlueprint.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/SimpleConstructionScript.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Utilities/BlueprintUtilities.h"

bool IBlueprintImporter::Import() {
	const UBlueprintGeneratedClass* BlueprintGeneratedClass = Create<UBlueprintGeneratedClass>();
	if (!BlueprintGeneratedClass) return false;

	Blueprint = UBlueprint::GetBlueprintFromClass(BlueprintGeneratedClass);
	if (!Blueprint) return false;

	FUObjectExport* Export = GetClassDefaultObject(AssetContainer, GetAssetDataAsValue());

	UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass);
	GetObjectSerializer()->DeserializeObjectProperties(Export->GetProperties(), GeneratedClass->GetDefaultObject());
	Export->Object = GeneratedClass;

	SetupConstructionScript();
	SetupWidgetTree();

	return OnAssetCreation(Blueprint);
}

void IBlueprintImporter::SetupConstructionScript() const {
	if (!GetAssetDataAsValue().Has("SimpleConstructionScript")) return;

	GetObjectSerializer()->bUseExperimentalSpawning = true;
	
	UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass);

	if (GeneratedClass->SimpleConstructionScript != nullptr) {
		MoveToTransientPackageAndRename(GeneratedClass->SimpleConstructionScript);
		MoveToTransientPackageAndRename(Blueprint->SimpleConstructionScript);
	}

	GeneratedClass->SimpleConstructionScript = NewObject<USimpleConstructionScript>(GeneratedClass);
	GeneratedClass->SimpleConstructionScript->SetFlags(RF_Transactional);

	Blueprint->SimpleConstructionScript = GeneratedClass->SimpleConstructionScript;

	USimpleConstructionScript* SimpleConstructionScript = GeneratedClass->SimpleConstructionScript;
	
	FUObjectExport* SimpleConstructionScriptExport = AssetContainer->GetExportByObjectPath(GetAssetDataAsValue().GetObject("SimpleConstructionScript"));
	SimpleConstructionScriptExport->Object = SimpleConstructionScript;
	
	GetObjectSerializer()->DeserializeObjectProperties(SimpleConstructionScriptExport->GetProperties(), SimpleConstructionScript);

	SimpleConstructionScript->FixupRootNodeParentReferences();
}

void IBlueprintImporter::SetupWidgetTree() {
	if (!GetAssetDataAsValue().Has("WidgetTree")) return;

	GetObjectSerializer()->bUseExperimentalSpawning = true;

	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Blueprint);
	
	FUObjectExport* WidgetTreeExport = AssetContainer->GetExportByObjectPath(GetAssetDataAsValue().GetObject("WidgetTree"));
	WidgetTreeExport->Object = WidgetBlueprint->WidgetTree;

	GetObjectSerializer()->SpawnExport(WidgetTreeExport, true);

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
}

UObject* IBlueprintImporter::CreateAsset(UObject* CreatedAsset) {
	UClass* Class = LoadClass(GetSuperStructJsonObject(GetAssetData()));
	
	if (!Class) {
		AppendNotification(
			FText::FromString("Blueprint Class Missing"),
			FText::FromString("The parent Blueprint's class could not be found. Ensure the class is defined."),
			2.0f,
			SNotificationItem::CS_Fail,
			true,
			350.0f
		);
		
		return nullptr;
	}
	
	/* Find the blueprint class and generated class */
	UClass* BlueprintClass = nullptr, *GeneratedClass = nullptr;
	
	FModuleManager::LoadModuleChecked<IKismetCompilerInterface>
		("KismetCompiler")
			.GetBlueprintTypesForClass(
				Class,
				BlueprintClass,
				GeneratedClass
			);

	if (UBlueprint* ExistingBlueprint = LoadObject<UBlueprint>(nullptr, *GetPackage()->GetPathName())) {
		FBlueprintEditorUtils::PropagateParentBlueprintDefaults(
			ExistingBlueprint->GeneratedClass
		);

		return IImporter::CreateAsset(ExistingBlueprint->GeneratedClass);
	}

	return IImporter::CreateAsset(FKismetEditorUtilities::CreateBlueprint(
		Class,
		GetPackage(),
		FName(*GetAssetName()),
		GetBlueprintType(Class),
		BlueprintClass,
		GeneratedClass
	)->GeneratedClass);
}
