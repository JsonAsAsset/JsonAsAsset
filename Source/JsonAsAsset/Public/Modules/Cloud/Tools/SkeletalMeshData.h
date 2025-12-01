/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/Tools/ToolBase.h"

class JSONASASSET_API TSkeletalMeshData : public TToolBase {
public:
	virtual void Execute();

protected:
	static TArray<FSkeletalMaterial> GetMaterials(USkeletalMesh* Mesh);
};