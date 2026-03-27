// Copyright (c) Gallant Lab. All Rights Reserved.
//
// Portions of this file are adapted from the CARLA open-source autonomous driving simulator,
// version 0.8.4 (https://github.com/carla-simulator/carla).
// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// Licensed under the MIT License (https://opensource.org/licenses/MIT).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sensor/MRIPostProcessEffect.h"
#include "MRIFrameCaptureCamera.generated.h"

class UTextureRenderTarget2D;
class USceneCaptureComponent2D;
class UStaticMeshComponent;
class UDrawFrustumComponent;
class UMaterial;

/**
 * An actor that captures rendered frames from the scene to disk or to memory.
 *
 * Attach to the replicated player pawn during demo playback to render frames
 * depth maps, semantic segmentation images, or surface normals during
 * demo playback. Configure the post-process mode, resolution, FOV, and
 * capture rate in the editor or at runtime.
 */
UCLASS(Blueprintable, BlueprintType, hidecategories = (Collision, Attachment, Actor))
class MRIEXPERIMENT_API AMRIFrameCaptureCamera : public AActor
{
	GENERATED_BODY()

public:
	/**
	 * Constructs the camera actor and creates all sub-components (scene capture,
	 * mesh frustum, render target).
	 * @param objectInitializer  Unreal object initializer forwarded to the parent class.
	 */
	AMRIFrameCaptureCamera(const FObjectInitializer& objectInitializer);

	/** Called after the actor is created. Initializes the capture render target. */
	virtual void PostActorCreated() override;
	/** Called when the game starts. Configures the capture component settings. */
	virtual void BeginPlay() override;
	/**
	 * Called when the actor is removed from the world. Releases the render target.
	 * @param endPlayReason  Reason the actor's EndPlay was triggered.
	 */
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
	/**
	 * Called every frame. Captures the scene if bCaptureScene is true and the
	 * configured CapturesPerSecond rate has elapsed.
	 * @param deltaSeconds  Time elapsed since the last frame in seconds.
	 */
	virtual void Tick(float deltaSeconds) override;

	/**
	 * Creates and configures the UTextureRenderTarget2D used as the capture destination.
	 * Must be called before ReadPixels or SaveCaptureToDisk.
	 */
	void InitCaptureRender();

	/**
	 * Returns the configured capture image width in pixels.
	 * @return Image width in pixels.
	 */
	uint32 GetImageSizeX() const;

	/**
	 * Returns the configured capture image height in pixels.
	 * @return Image height in pixels.
	 */
	uint32 GetImageSizeY() const;

	/**
	 * Returns the active post-process rendering mode.
	 * @return The EMRIPostProcessEffect applied to captured frames.
	 */
	EMRIPostProcessEffect GetPostProcessEffect() const;

	/**
	 * Returns the camera's horizontal field-of-view angle.
	 * @return FOV angle in degrees.
	 */
	float GetFOVAngle() const;

	/**
	 * Sets the capture resolution.
	 * @param newSizeX  Desired image width in pixels.
	 * @param newSizeY  Desired image height in pixels.
	 */
	void SetImageSize(uint32 newSizeX, uint32 newSizeY);

	/**
	 * Applies the given post-process effect to all subsequently captured frames.
	 * @param effect  The EMRIPostProcessEffect to apply.
	 */
	void SetPostProcessEffect(EMRIPostProcessEffect effect);

	/**
	 * Sets the horizontal field-of-view angle.
	 * @param angle  FOV in degrees.
	 */
	void SetFOVAngle(float angle);

	/**
	 * Sets the target gamma for captured images.
	 * @param gamma  Gamma correction value (e.g. 2.2 for standard sRGB).
	 */
	void SetTargetGamma(float gamma);

	/**
	 * Reads the most recently captured frame into a pixel array.
	 * @param bitMap  Output array filled with RGBA pixel data.
	 * @return true if pixels were read successfully.
	 */
	bool ReadPixels(TArray<FColor>& bitMap) const;

	/**
	 * Captures the current scene and saves it to a PNG file at the given path.
	 * @param filePath  Absolute path of the output PNG file.
	 * @return true if the file was written successfully.
	 */
	UFUNCTION(BlueprintCallable)
	bool SaveCaptureToDisk(const FString& filePath);

	/**
	 * Requests that the next available frame be saved to the configured folder.
	 * @return true if the save request was accepted.
	 */
	bool SaveNextFrame();

	/**
	 * Sets the folder where automatically captured frames are saved.
	 * @param folder  Absolute path to the output directory.
	 */
	void SetSaveFolder(const FString& folder);

	/**
	 * Attaches this camera to the given actor so it follows its transform.
	 * @param targetActor  The actor to attach to.
	 */
	void AttachToActor(AActor* targetActor);

public:
	/** If true, the camera captures a frame every tick (rate-limited by CapturesPerSecond). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCaptureScene = false;

	/** Folder path where captured frames are written when bCaptureScene is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SaveToFolder;

	/** Base filename prefix used for saved frame files. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FileName;

	/** Maximum number of frames per second to capture. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CapturesPerSecond = 10.0f;

private:
	/** Capture image width in pixels. */
	UPROPERTY(EditAnywhere)
	uint32 sizeX;

	/** Capture image height in pixels. */
	UPROPERTY(EditAnywhere)
	uint32 sizeY;

	/** Post-process effect applied to captured images. */
	UPROPERTY(EditAnywhere)
	EMRIPostProcessEffect postProcessEffect;

	/** Static mesh component used to visualize the camera frustum in the editor. */
	UPROPERTY()
	UStaticMeshComponent* meshComponent;

	/** Frustum helper component for editor visualization. */
	UPROPERTY()
	UDrawFrustumComponent* drawFrustum;

	/** Render target that the scene capture component writes to. */
	UPROPERTY()
	UTextureRenderTarget2D* captureRenderTarget;

	/** The Unreal scene capture component that drives rendering to the render target. */
	UPROPERTY(EditAnywhere)
	USceneCaptureComponent2D* captureComponent2D;

	/** Material applied to the capture component to produce depth images. */
	UPROPERTY()
	UMaterial* postProcessDepth;

	/** Material applied to the capture component to produce semantic segmentation images. */
	UPROPERTY()
	UMaterial* postProcessSemanticSegmentation;

	/** Material applied to the capture component to produce world-space normal images. */
	UPROPERTY()
	UMaterial* postProcessWorldNormal;

	/** Material applied to the capture component to produce camera-space normal images. */
	UPROPERTY()
	UMaterial* postProcessCameraNormal;

	/** Running counter used to generate unique filenames for each saved frame. */
	uint32 captureFileNameCount;
};
