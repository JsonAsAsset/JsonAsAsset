/* Copyright JsonAsAsset Contributors 2024-2026 */

#include "Importers/Constructor/TypesHelper.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Importers/Constructor/Types.h"
#include "Importers/Constructor/Registry/RegistrationInfo.h"
#include "Utilities/Compatibility.h"

bool CanImport(const FString& Type, const bool IsCloud, const UClass* Class) {
	if (IsCloud) {
		if (!ImportTypes::Cloud::Allowed(Type)) {
			return false;
		}
	}
    
	if (FindFactoryForAssetType(Type)) {
		return true;
	}
    
	for (const TPair<FString, TArray<FString>>& Pair : ImportTypes::Templated) {
		if (Pair.Value.Contains(Type)) {
			return true;
		}
	}

	if (!Class) {
		Class = FindClassByType(Type);
	}

	if (Class == nullptr) return false;

	/* ?? */
	if (Type == "MaterialInterface") return true;

	if (ImportTypes::Cloud::Extra.Contains(Type)) {
		return true;
	}

	if (!ImportTypes::Allowed(Type)) return false;
	if (Class->IsChildOf(UDataAsset::StaticClass())) return true;

	if (Class->IsChildOf(UTexture::StaticClass())) return false;

	/*/* If the Class has an asset type action, it's importable #1#
	const FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	const IAssetTools& AssetTools = AssetToolsModule.Get();
	
	static TArray<TWeakPtr<IAssetTypeActions>> OutAssetTypeActionsList = {};
	AssetTools.GetAssetTypeActionsList(OutAssetTypeActionsList);

	for (TWeakPtr AssetTypeAction : OutAssetTypeActionsList) {
		if (const TSharedPtr<IAssetTypeActions> Action = AssetTypeAction.Pin()) {
			const UClass* SupportedClass = Action->GetSupportedClass();

			if (SupportedClass == Class) return true;
		}
	}*/

	return false;
}
