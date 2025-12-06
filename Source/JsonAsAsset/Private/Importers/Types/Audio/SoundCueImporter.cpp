/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Types/Audio/SoundCueImporter.h"
#include "Sound/SoundCue.h"

UObject* ISoundCueImporter::CreateAsset(UObject* CreatedAsset) {
	return IImporter::CreateAsset(NewObject<USoundCue>(GetPackage(), *GetAssetName(), RF_Public | RF_Standalone));
}

bool ISoundCueImporter::Import() {
	USoundCue* SoundCue = Create<USoundCue>();
	
	SoundCue->PreEditChange(nullptr);
	
	/* Start importing nodes ~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	if (SoundCue) {
		TMap<FString, USoundNode*> SoundCueNodes;
		
		ConstructNodes(SoundCue, JsonObjects, SoundCueNodes);
		SetupNodes(SoundCue, SoundCueNodes, JsonObjects);
	}
	/* End of importing nodes ~~~~~~~~~~~~~~~~~~~~~~~~~~ */

	GetObjectSerializer()->DeserializeObjectProperties(RemovePropertiesShared(GetAssetData(), TArray<FString> {
		"FirstNode"
	}), SoundCue);
	
	SoundCue->PostEditChange();
	SoundCue->CompileSoundNodesFromGraphNodes();

	return OnAssetCreation(SoundCue);
}
