/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Importers/Constructor/Graph/SoundGraph.h"

class ISoundCueImporter : public ISoundGraph {
public:
	ISoundCueImporter(const TSharedPtr<FJsonObject>& JsonObject, UPackage* Package, const TArray<TSharedPtr<FJsonValue>>& AllJsonObjects):
		ISoundGraph(JsonObject, Package, AllJsonObjects) {
	}

	virtual UObject* CreateAsset(UObject* CreatedAsset) override;
	virtual bool Import() override;
};

REGISTER_IMPORTER(ISoundCueImporter, {
	"SoundCue"
}, "Sound Assets");