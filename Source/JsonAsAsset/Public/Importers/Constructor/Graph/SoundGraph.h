/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Importer.h"
#include "Interfaces/IHttpRequest.h"
#include "Sound/SoundNodeWavePlayer.h"

/*
 * Sound Graph Handler
 * Handles everything needed to create a sound graph from JSON.
*/
class ISoundGraph : public IImporter {
public:
	ISoundGraph(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& JsonObjects):
		IImporter(JsonObject, Package, JsonObjects) {
	}

protected:
	/* Graph Functions */
	static void ConnectEdGraphNode(UEdGraphNode* NodeToConnect, UEdGraphNode* NodeToConnectTo, int Pin);
	static void ConnectSoundNode(const USoundNode* NodeToConnect, const USoundNode* NodeToConnectTo, int Pin);

	/* Creates an empty USoundNode */
	static USoundNode* CreateEmptyNode(FName Name, FName Type, USoundCue* SoundCue);

	static void ConstructNodes(USoundCue* SoundCue, TArray<TSharedPtr<FJsonValue>> JsonArray, TMap<FString, USoundNode*>& OutNodes);
	void SetupNodes(USoundCue* SoundCueAsset, TMap<FString, USoundNode*> SoundCueNodes, TArray<TSharedPtr<FJsonValue>> JsonObjectArray) const;

	/* Sound Wave Import */
	static void OnDownloadSoundWave(FString SavePath, FString AssetPtr, USoundNodeWavePlayer* Node);
};
