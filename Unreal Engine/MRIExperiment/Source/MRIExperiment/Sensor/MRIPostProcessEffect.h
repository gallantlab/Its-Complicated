// MRI Experiment Plugin - Post Process Effect Enum

#pragma once

#include "MRIPostProcessEffect.generated.h"

/**
 * Specifies the post-process rendering mode applied by a frame-capture camera sensor.
 *
 * Selected at design time on AMRIFrameCaptureCamera to control what visual information
 * is written to disk when the camera captures a frame.
 */
UENUM(BlueprintType)
enum class EMRIPostProcessEffect : uint8
{
	/** Capture the raw RGB image with no post-processing applied. */
	None                  UMETA(DisplayName = "RGB without any post-processing"),
	/** Capture the final RGB image including all scene post-processing. */
	SceneFinal            UMETA(DisplayName = "RGB with post-processing present at the scene"),
	/** Capture a depth map (distance from the camera to each pixel). */
	Depth                 UMETA(DisplayName = "Depth Map"),
	/** Capture a semantic segmentation image (each object class as a distinct color). */
	SemanticSegmentation  UMETA(DisplayName = "Semantic Segmentation"),
	/** Capture per-pixel surface normals in world space. */
	WorldNormal           UMETA(DisplayName = "World Normals"),
	/** Capture per-pixel surface normals in camera space. */
	CameraNormal          UMETA(DisplayName = "Camera Normals"),

	/** Sentinel value representing the total number of valid enum entries. */
	SIZE                  UMETA(Hidden),
	/** Sentinel value used to indicate an uninitialized or invalid mode. */
	INVALID               UMETA(Hidden),
};
