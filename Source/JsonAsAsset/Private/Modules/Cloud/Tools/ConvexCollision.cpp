/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Modules/Cloud/Tools/ConvexCollision.h"

#include "Engine/StaticMeshSocket.h"
#include "Utilities/EngineUtilities.h"

#include "PhysicsEngine/BodySetup.h"

void TToolConvexCollision::Execute() {
	TArray<FAssetData> AssetDataList = GetAssetsInSelectedFolder();

	if (AssetDataList.Num() == 0) {
		return;
	}

	for (const FAssetData& AssetData : AssetDataList) {
		if (!AssetData.IsValid()) continue;
		if (AssetData.AssetClass != "StaticMesh") continue;
		
		UObject* Asset = AssetData.GetAsset();
		if (Asset == nullptr) continue;
		
		UStaticMesh* StaticMesh = Cast<UStaticMesh>(Asset);
		if (StaticMesh == nullptr) continue;

		/* Request to API ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
		FString ObjectPath = AssetData.ObjectPath.ToString();

		const TSharedPtr<FJsonObject> Response = Cloud::Export::GetRaw(ObjectPath);
		if (Response == nullptr || ObjectPath.IsEmpty()) continue;

		/* Not found */
		if (Response->HasField(TEXT("errored"))) {
			continue;
		}

		TArray<TSharedPtr<FJsonValue>> Exports = Response->GetArrayField(TEXT("exports"));
		
		/* Get Body Setup (different in Unreal Engine versions) ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#if !UE4_27_ONLY_BELOW
		UBodySetup* BodySetup = StaticMesh->GetBodySetup();
#else
		UBodySetup* BodySetup = StaticMesh->BodySetup;
#endif

		for (const TSharedPtr<FJsonValue>& Export : Exports) {
			if (!Export.IsValid() || !Export->AsObject().IsValid()) {
				continue;
			}

			const TSharedPtr<FJsonObject> JsonObject = Export->AsObject();
			if (!IsProperExportData(JsonObject)) continue;

			TSharedPtr<FJsonObject> Properties = JsonObject->GetObjectField(TEXT("Properties"));
			FString Type = JsonObject->GetStringField(TEXT("Type"));

			if (Type == "StaticMesh") {
				StaticMesh->DistanceFieldSelfShadowBias = 0.0;

				/* Create an object serializer */
				StaticMesh->Sockets.Empty();

				GetObjectSerializer()->SetExportForDeserialization(JsonObject, StaticMesh);
				GetObjectSerializer()->Parent = StaticMesh;

				GetObjectSerializer()->DeserializeExports(Exports);

				for (const FUObjectExport UObjectExport : GetObjectSerializer()->GetPropertySerializer()->ExportsContainer.Exports) {
					if (UStaticMeshSocket* Socket = Cast<UStaticMeshSocket>(UObjectExport.Object)) {
						StaticMesh->AddSocket(Socket);
					}
				}
				
				GetObjectSerializer()->DeserializeObjectProperties(RemovePropertiesShared(Properties, {
					"StaticMaterials",
					"Sockets"
				}), StaticMesh);
			}

			/* Check if the Class matches BodySetup */
			if (Type != "BodySetup") continue;
			
			/* Empty any collision data */
			BodySetup->AggGeom.EmptyElements();
			BodySetup->CollisionTraceFlag = CTF_UseDefault;

			GetObjectSerializer()->DeserializeObjectProperties(Properties, BodySetup);

			BodySetup->PostEditChange();

			/* Update physics data */
			BodySetup->InvalidatePhysicsData();
			BodySetup->CreatePhysicsMeshes();

			StaticMesh->MarkPackageDirty();
			StaticMesh->Modify(true);

			/* Notification */
			AppendNotification(
				FText::FromString("Imported Convex Collision: " + StaticMesh->GetName()),
				FText::FromString(StaticMesh->GetName()),
				3.5f,
				FAppStyle::GetBrush("PhysicsAssetEditor.EnableCollision.Small"),
				SNotificationItem::CS_Success,
				false,
				310.0f
			);
		}
	}
}
