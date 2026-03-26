/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Importers/Types/Blueprint/BlueprintImporter.h"

#include "KismetCompilerModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Utilities/BlueprintUtilities.h"

bool IBlueprintImporter::Import() {
	UBlueprint* Blueprint = Create<UBlueprint>();
	if (!Blueprint) return false;

	const FUObjectExport& Export = GetClassDefaultObject(AssetContainer, GetAssetDataAsValue());

	const UBlueprintGeneratedClass* GeneratedClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass);
	GetObjectSerializer()->DeserializeObjectProperties(Export.GetProperties(), GeneratedClass->GetDefaultObject());

	return OnAssetCreation(Blueprint);
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

	if (UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *GetPackage()->GetPathName())) {
		FBlueprintEditorUtils::PropagateParentBlueprintDefaults(
			Blueprint->GeneratedClass
		);

		return Blueprint;
	}

	return FKismetEditorUtilities::CreateBlueprint(
		Class,
		GetPackage(),
		FName(*GetAssetName()),
		GetBlueprintType(Class),
		BlueprintClass,
		GeneratedClass
	);
}