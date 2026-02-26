/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Modules/EngineFactory.h"
#include "Importers/Constructor/ImportReader.h"
#include "Modules/Toolbar/Toolbar.h"

UJEngineImplementation::UJEngineImplementation() {
	Formats.Add("json;Plugin");
	SupportedClass = UObject::StaticClass();

	bEditorImport = true;
	bText = false;
}

bool UJEngineImplementation::FactoryCanImport(const FString& Filename) {
	return false;
}

UObject* UJEngineImplementation::Import(const FString& Filename) {
	/* For now. */
	return nullptr;

	IImporter* Importer = IImportReader::ImportReference(Filename);
	return Importer->GetAsset();
}

UObject* UJEngineImplementation::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Params, FFeedbackContext* Warn, bool& bOutOperationCanceled) {
	return Import(Filename);
}