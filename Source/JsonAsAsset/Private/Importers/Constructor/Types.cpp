/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Constructor/Types.h"

#include "Settings/JsonAsAssetSettings.h"
#include "Utilities/EngineUtilities.h"

/* Define supported template asset class here */
TMap<FString, TArray<FString>> ImportTypes::Templated = {
	{
		TEXT("Curve Assets"),
		{
			TEXT("CurveFloat"),
			TEXT("CurveLinearColor"),
		}
	},
	{
		TEXT("Data Assets"),
		{
			TEXT("SlateBrushAsset"),
			TEXT("SlateWidgetStyleAsset"),
			TEXT("AnimBoneCompressionSettings"),
			TEXT("AnimCurveCompressionSettings"),
			TEXT("LandscapeLayerInfoObject"),
			TEXT("HLODProxy")
		}
	},
	{
		TEXT("Landscape Assets"),
		{
			TEXT("LandscapeGrassType"),
			TEXT("FoliageType_InstancedStaticMesh"),
			TEXT("FoliageType_Actor"),
		}
	},
	{
		TEXT("Material Assets"),
		{
			TEXT("MaterialParameterCollection"),
			TEXT("SubsurfaceProfile"),
		}
	},
	{
		TEXT("Skeletal Assets"),
		{
			TEXT("SkeletalMeshLODSettings"),
		}
	},
	{
		TEXT("Physics Assets"),
		{
			TEXT("PhysicalMaterial"),
		}
	},
	{
		TEXT("Sound Assets"),
		{
			TEXT("ReverbEffect"),
			TEXT("SoundAttenuation"),
			TEXT("SoundConcurrency"),
			TEXT("SoundClass"),
			TEXT("SoundMix"),
			TEXT("SoundModulationPatch"),
			TEXT("SubmixEffectDynamicsProcessorPreset"),
		}
	},
	{
		TEXT("Texture Assets"),
		{
			TEXT("TextureRenderTarget2D"),
			TEXT("RuntimeVirtualTexture"),
		}
	},
	{
		TEXT("Sequencer Assets"),
		{
			TEXT("CameraAnim"),
		}
	}
};

bool ImportTypes::Cloud::Allowed(const FString& Type) {
	if (Blacklisted.Contains(Type)) {
		return false;
	}

	if (Extra.Contains(Type)) {
		return true;
	}

	return true;
}

bool ImportTypes::Allowed(const FString& ImporterType) {
	if (Experimental.Contains(ImporterType)) {
		const UJsonAsAssetSettings* Settings = GetSettings();

		if (!Settings->bEnableExperiments) return false;
	}

	return true;
}
