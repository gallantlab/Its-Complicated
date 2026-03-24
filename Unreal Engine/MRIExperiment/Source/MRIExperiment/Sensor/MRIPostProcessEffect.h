// MRI Experiment Plugin - Post Process Effect Enum

#pragma once

#include "MRIPostProcessEffect.generated.h"

UENUM(BlueprintType)
enum class EMRIPostProcessEffect : uint8
{
	None                  UMETA(DisplayName = "RGB without any post-processing"),
	SceneFinal            UMETA(DisplayName = "RGB with post-processing present at the scene"),
	Depth                 UMETA(DisplayName = "Depth Map"),
	SemanticSegmentation  UMETA(DisplayName = "Semantic Segmentation"),
	WorldNormal           UMETA(DisplayName = "World Normals"),
	CameraNormal          UMETA(DisplayName = "Camera Normals"),

	SIZE                  UMETA(Hidden),
	INVALID               UMETA(Hidden),
};
