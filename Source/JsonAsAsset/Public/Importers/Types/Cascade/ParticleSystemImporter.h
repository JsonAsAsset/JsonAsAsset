/* Copyright JsonAsAsset Contributors 2024-2026 */

#pragma once

#include "Importers/Constructor/Importer.h"
#include "Particles/ParticleEmitter.h"

class IParticleSystemImporter : public IImporter {
public:
	virtual UObject* CreateAsset(UObject* CreatedAsset) override;
	virtual bool Import() override;

private:
	void CreateDistributions();
	void WipeEmitters() const;

	void CreateEmitters(const TArray<FUObjectJsonValueExport>& Exports);
	UParticleEmitter* CreateEmitter(const UClass* Class, FName Name, const TSharedPtr<FJsonObject>& EmitterProperties);

	UParticleLODLevel* CreateLODLevel(const TSharedPtr<FJsonObject>& LevelProperties, UParticleEmitter* Emitter);
	void DeserializeModule(const TSharedPtr<FJsonObject>& ModuleProperties, UParticleModule* Module) const;
};

REGISTER_IMPORTER(IParticleSystemImporter, {
	"ParticleSystem"
}, "Cascade Assets");