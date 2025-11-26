/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "ToolBase.h"
#include "SkeletalMeshData.generated.h"

USTRUCT()
struct FSkeletalMeshData : public FToolBase {
	GENERATED_BODY()
public:
	void Execute();

protected:
	static TArray<FSkeletalMaterial> GetMaterials(USkeletalMesh* Mesh);
};