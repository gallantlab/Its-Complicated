// Copyright (c) Gallant Lab. All Rights Reserved.
//
// Portions of this file are adapted from the CARLA open-source autonomous driving simulator,
// version 0.8.4 (https://github.com/carla-simulator/carla).
// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// Licensed under the MIT License (https://opensource.org/licenses/MIT).

#pragma once

#include "ExperimentPostProcessEffect.generated.h"

/**
 * Specifies the post-process rendering mode applied by a frame-capture camera.
 *
 * Selected at design time on AExperimentFrameCaptureCamera to control what visual information
 * is written to disk when the camera captures a frame.
 */
UENUM(BlueprintType)
enum class EExperimentPostProcessEffect : uint8
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

	/** Total number of valid enum entries. */
	SIZE                  UMETA(Hidden),
	/** Invalid mode. */
	INVALID               UMETA(Hidden),
};

/**
 * Parses an EExperimentPostProcessEffect value from its name in a settings file.
 * @param String  The string representation of the effect.
 * @return The matching effect, or SceneFinal if unrecognized.
 */
inline EExperimentPostProcessEffect PostProcessEffectFromString(const FString& String)
{
	if (String == TEXT("None"))                  return EExperimentPostProcessEffect::None;
	if (String == TEXT("SceneFinal"))            return EExperimentPostProcessEffect::SceneFinal;
	if (String == TEXT("Depth"))                 return EExperimentPostProcessEffect::Depth;
	if (String == TEXT("SemanticSegmentation"))  return EExperimentPostProcessEffect::SemanticSegmentation;
	if (String == TEXT("WorldNormal"))           return EExperimentPostProcessEffect::WorldNormal;
	if (String == TEXT("CameraNormal"))          return EExperimentPostProcessEffect::CameraNormal;
	return EExperimentPostProcessEffect::SceneFinal;
}

/**
 * Returns the settings-file name for the given effect.
 * @param Effect  The effect to convert.
 * @return The string representation of the effect.
 */
inline FString PostProcessEffectToString(EExperimentPostProcessEffect Effect)
{
	switch (Effect)
	{
		case EExperimentPostProcessEffect::None:                  return TEXT("None");
		case EExperimentPostProcessEffect::SceneFinal:            return TEXT("SceneFinal");
		case EExperimentPostProcessEffect::Depth:                 return TEXT("Depth");
		case EExperimentPostProcessEffect::SemanticSegmentation:  return TEXT("SemanticSegmentation");
		case EExperimentPostProcessEffect::WorldNormal:           return TEXT("WorldNormal");
		case EExperimentPostProcessEffect::CameraNormal:          return TEXT("CameraNormal");
		default:                                                  return TEXT("SceneFinal");
	}
}
