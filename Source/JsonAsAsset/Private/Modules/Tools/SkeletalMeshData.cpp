/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Modules/Tools/SkeletalMeshData.h"

#include <string>

#include "ClothingAssetBase.h"
#include "Utilities/EngineUtilities.h"

#include "Dom/JsonObject.h"
#include "Animation/AnimSequence.h"

#if ENGINE_UE5 && ENGINE_MINOR_VERSION >= 3
#include "ClothingAsset.h"
#include "ClothLODData.h"
#else
#include "ClothingSystemRuntimeCommon/Public/ClothingAsset.h"
#endif

#include "Engine/SkeletalMeshSocket.h"

#if ENGINE_UE5
#include "Engine/SkinnedAssetCommon.h"
#endif

#include "Importers/Constructor/Importer.h"

#if ENGINE_UE5
#include "Animation/AnimData/IAnimationDataController.h"
#if ENGINE_MINOR_VERSION >= 4
#include "Animation/AnimData/IAnimationDataModel.h"
#endif
#include "AnimDataController.h"
#endif

class UClothingAssetCommon;

void FSkeletalMeshData::Execute() {
	TArray<FAssetData> AssetDataList = GetAssetsInSelectedFolder();

	USkeletalMesh* SkeletalMeshSelected = GetSelectedAsset<USkeletalMesh>(true);

	if (AssetDataList.Num() == 0 && SkeletalMeshSelected == nullptr) {
		return;
	}

	if (SkeletalMeshSelected) {
		AssetDataList.Empty();
		
		FAssetData AssetData(SkeletalMeshSelected);
		AssetDataList.Add(AssetData);
	}

	for (const FAssetData& AssetData : AssetDataList) {
		if (!AssetData.IsValid()) continue;
		if (AssetData.AssetClass != "SkeletalMesh") continue;
		
		UObject* Asset = AssetData.GetAsset();
		if (Asset == nullptr) continue;
		
		USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(Asset);
		if (SkeletalMesh == nullptr) continue;

		/* Request to API ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
		FString ObjectPath = AssetData.ObjectPath.ToString();

		const TSharedPtr<FJsonObject> Response = FAssetUtilities::API_RequestExports(ObjectPath);
		if (Response == nullptr || ObjectPath.IsEmpty()) continue;

		/* Not found */
		if (Response->HasField(TEXT("errored"))) {
			continue;
		}

		TArray<TSharedPtr<FJsonValue>> Exports = Response->GetArrayField(TEXT("jsonOutput"));
		
		for (const TSharedPtr<FJsonValue>& Export : Exports) {
			if (!Export.IsValid() || !Export->AsObject().IsValid()) {
				continue;
			}

			const TSharedPtr<FJsonObject> JsonObject = Export->AsObject();
			if (!IsProperExportData(JsonObject)) continue;

			TSharedPtr<FJsonObject> Properties = JsonObject->GetObjectField(TEXT("Properties"));
			const FString Type = JsonObject->GetStringField(TEXT("Type"));
			const FString Name = JsonObject->GetStringField(TEXT("Name"));

			if (Name != Asset->GetName()) continue;

			if (Type == "SkeletalMesh") {
				bool UseClothingAssets = false;

				TArray<UClothingAssetBase*> ClothingAssets = SkeletalMesh->GetMeshClothingAssets();

				if (UseClothingAssets) {
					/* Empty all Clothing Assets */
					for (UClothingAssetBase* ClothingAsset : ClothingAssets) {
						ClothingAsset->Modify();
						ClothingAsset->UnbindFromSkeletalMesh(SkeletalMesh, 0);
						SkeletalMesh->GetMeshClothingAssets().Remove(ClothingAsset);
					}
				}

				TArray<TSharedPtr<FJsonValue>> SkeletalMaterials = JsonObject->GetArrayField("SkeletalMaterials");

				int SkeletalMaterialIndex = 0;
				
				for (const TSharedPtr<FJsonValue>& SkeletalMaterialExport : SkeletalMaterials) {
					if (!SkeletalMaterialExport.IsValid() || !SkeletalMaterialExport->AsObject().IsValid()) {
						continue;
					}

					const TSharedPtr<FJsonObject> SkeletalMaterialObject = SkeletalMaterialExport->AsObject();

					if (SkeletalMesh->GetMaterials().IsValidIndex(SkeletalMaterialIndex)) {
						FSkeletalMaterial& MaterialSlot = SkeletalMesh->GetMaterials()[SkeletalMaterialIndex];
						
						MaterialSlot.MaterialSlotName = FName(*SkeletalMaterialObject->GetStringField("MaterialSlotName"));
						MaterialSlot.ImportedMaterialSlotName = MaterialSlot.MaterialSlotName;

						TSharedPtr<FJsonObject> SkeletalMaterial = SkeletalMaterialObject->GetObjectField("Material");

						IImporter* Importer = new IImporter();
						
						TObjectPtr<UObject> LoadedObject;
						Importer->LoadObject<UObject>(&SkeletalMaterial, LoadedObject);

						if (IsObjectPtrValid(LoadedObject)) MaterialSlot.MaterialInterface = Cast<UMaterialInterface>(LoadedObject.Get());
					} else break;

					SkeletalMaterialIndex++;
				}

				/* Create an object serializer */
				UObjectSerializer* ObjectSerializer = CreateObjectSerializer();

				ObjectSerializer->SetExportForDeserialization(JsonObject, SkeletalMesh);
				ObjectSerializer->ParentAsset = SkeletalMesh;
				
				SkeletalMesh->GetMeshOnlySocketList().Empty();

				ObjectSerializer->DeserializeExports(Exports);

				for (const FUObjectExport UObjectExport : ObjectSerializer->GetPropertySerializer()->ExportsContainer.Exports) {
					if (USkeletalMeshSocket* Socket = Cast<USkeletalMeshSocket>(UObjectExport.Object)) {
						SkeletalMesh->GetMeshOnlySocketList().Add(Socket);
					}
				}

				ObjectSerializer->DeserializeObjectProperties(KeepPropertiesShared(Properties, {
					// "MeshClothingAssets"
					"PhysicsAsset",
					"PostProcessAnimBlueprint",
					"ShadowPhysicsAsset",
					"PositiveBoundsExtension",
					"NegativeBoundsExtension",

					"Sockets"
				}), SkeletalMesh);
				
				SkeletalMesh->Modify();
				
				if (UseClothingAssets) {
					ClothingAssets = SkeletalMesh->GetMeshClothingAssets();
				
					for (UClothingAssetBase* ClothingAssetBase : ClothingAssets) {
						ClothingAssetBase->Modify();

						if (UClothingAssetCommon* ClothingAsset = Cast<UClothingAssetCommon>(ClothingAssetBase)) {
							for (FClothLODDataCommon& LodData : ClothingAsset->LodData) {
								LodData.PointWeightMaps.Empty();

								for (TMap<uint32, FPointWeightMap>::TConstIterator Iterator(LodData.PhysicalMeshData.WeightMaps); Iterator; ++Iterator) {
									const uint32 Key = Iterator.Key();
									FPointWeightMap PointWeightMap = Iterator.Value();

									PointWeightMap.Name = FName(*FString::FromInt(Key));
									PointWeightMap.CurrentTarget = 1;
									LodData.PointWeightMaps.Add(PointWeightMap);
								}
							}
						}
					}
				}

				const TArray<FAssetData>& Assets = { Asset };
				const FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
				ContentBrowserModule.Get().SyncBrowserToAssets(Assets);

				/* Notification */
				AppendNotification(
					FText::FromString("Imported Skeletal Mesh Data: " + SkeletalMesh->GetName()),
					FText::FromString(SkeletalMesh->GetName()),
					3.5f,
					FAppStyle::GetBrush("PhysicsAssetEditor.EnableCollision.Small"),
					SNotificationItem::CS_Success,
					false,
					310.0f
				);
			}
		}
	}
}
