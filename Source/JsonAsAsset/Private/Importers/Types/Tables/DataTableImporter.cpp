/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/Tables/DataTableImporter.h"

bool IDataTableImporter::Import() {
	UDataTable* DataTable = NewObject<UDataTable>(Package, UDataTable::StaticClass(), *AssetName, RF_Public | RF_Standalone);
	
	/* ScriptClass for the Data Table */
	FString TableStruct; {
		/* --- Properties --> RowStruct --> ObjectName */
		/* --- Class'StructClass' --> StructClass */
		AssetData->GetObjectField(TEXT("RowStruct"))
			->GetStringField(TEXT("ObjectName")).
			Split("'", nullptr, &TableStruct);
		TableStruct.Split("'", &TableStruct, nullptr);
	}

	/* Find Table Row Struct */
#if UE5_7_BEYOND
	UScriptStruct* TableRowStruct = FindFirstObject<UScriptStruct>(*TableStruct); {
#else
	UScriptStruct* TableRowStruct = FindObject<UScriptStruct>(ANY_PACKAGE, *TableStruct); {
#endif
		if (TableRowStruct == nullptr) {
			AppendNotification(FText::FromString("DataTable Struct Missing: " + TableStruct), FText::FromString("You need the parent's data table structure defined exactly where it's supposed to."), 2.0f, SNotificationItem::CS_Fail, true, 350.0f);

			return false;
		} else {
			DataTable->RowStruct = TableRowStruct;
		}
	}

	/* Access Property Serializer */
	UPropertySerializer* ObjectPropertySerializer = GetObjectSerializer()->GetPropertySerializer();
	TSharedPtr<FJsonObject> RowData = AssetData->GetObjectField(TEXT("Rows"));

	/* Loop throughout row data, and deserialize */
	for (TPair<FString, TSharedPtr<FJsonValue>>& Pair : RowData->Values) {
		TSharedPtr<FStructOnScope> ScopedStruct = MakeShareable(new FStructOnScope(TableRowStruct));
		TSharedPtr<FJsonObject> StructData = Pair.Value->AsObject();

		/* Deserialize, add row */
		ObjectPropertySerializer->DeserializeStruct(TableRowStruct, StructData.ToSharedRef(), ScopedStruct->GetStructMemory());
		DataTable->AddRow(*Pair.Key, *reinterpret_cast<const FTableRowBase*>(ScopedStruct->GetStructMemory()));
	}

	/* Handle edit changes, and add it to the content browser */
	return OnAssetCreation(DataTable);
}
