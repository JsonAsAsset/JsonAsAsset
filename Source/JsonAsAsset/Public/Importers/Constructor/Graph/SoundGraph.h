/* Copyright JsonAsAsset Contributors 2024-2026 */

#pragma once

#include "Importers/Constructor/Importer.h"
#include "Sound/SoundNodeWavePlayer.h"

/* Handles everything needed to create a sound graph from JSON. */
class ISoundGraph : public IImporter {
protected:
	/* Graph Functions */
	static void ConnectEdGraphNode(UEdGraphNode* NodeToConnect, UEdGraphNode* NodeToConnectTo, int Pin);
	static void ConnectSoundNode(const USoundNode* NodeToConnect, const USoundNode* NodeToConnectTo, int Pin);

	/* Creates an empty USoundNode */
	static USoundNode* CreateEmptyNode(FName Name, FName Type, USoundCue* SoundCue);

	void ConstructNodes(USoundCue* SoundCue, TMap<FString, USoundNode*>& OutNodes);
	void SetupNodes(USoundCue* SoundCueAsset, TMap<FString, USoundNode*> SoundCueNodes) const;

	/* Sound Wave Import */
	static void OnDownloadSoundWave(FString SavePath, FString AssetPtr, USoundNodeWavePlayer* Node);
};
