/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "ToolBase.h"

struct FSkeletalMeshData : FToolBase {
	static void Execute();

	static TArray<FSkeletalMaterial> GetMaterials(USkeletalMesh* Mesh);
};