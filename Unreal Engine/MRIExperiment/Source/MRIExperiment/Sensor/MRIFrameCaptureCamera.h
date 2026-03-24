// MRI Experiment Plugin - Frame Capture Camera

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

UCLASS(Blueprintable, BlueprintType, hidecategories = (Collision, Attachment, Actor))
class MRIEXPERIMENT_API AMRIFrameCaptureCamera : public AActor
{
	GENERATED_BODY()

public:
	AMRIFrameCaptureCamera(const FObjectInitializer& objectInitializer);

	virtual void PostActorCreated() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
	virtual void Tick(float deltaSeconds) override;

	void InitCaptureRender();

	uint32 GetImageSizeX() const;
	uint32 GetImageSizeY() const;
	EMRIPostProcessEffect GetPostProcessEffect() const;
	float GetFOVAngle() const;

	void SetImageSize(uint32 newSizeX, uint32 newSizeY);
	void SetPostProcessEffect(EMRIPostProcessEffect effect);
	void SetFOVAngle(float angle);
	void SetTargetGamma(float gamma);

	bool ReadPixels(TArray<FColor>& bitMap) const;

	UFUNCTION(BlueprintCallable)
	bool SaveCaptureToDisk(const FString& filePath);

	bool SaveNextFrame();

	void SetSaveFolder(const FString& folder);

	void AttachToActor(AActor* targetActor);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCaptureScene = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SaveToFolder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CapturesPerSecond = 10.0f;

private:
	UPROPERTY(EditAnywhere)
	uint32 sizeX;

	UPROPERTY(EditAnywhere)
	uint32 sizeY;

	UPROPERTY(EditAnywhere)
	EMRIPostProcessEffect postProcessEffect;

	UPROPERTY()
	UStaticMeshComponent* meshComponent;

	UPROPERTY()
	UDrawFrustumComponent* drawFrustum;

	UPROPERTY()
	UTextureRenderTarget2D* captureRenderTarget;

	UPROPERTY(EditAnywhere)
	USceneCaptureComponent2D* captureComponent2D;

	UPROPERTY()
	UMaterial* postProcessDepth;

	UPROPERTY()
	UMaterial* postProcessSemanticSegmentation;

	UPROPERTY()
	UMaterial* postProcessWorldNormal;

	UPROPERTY()
	UMaterial* postProcessCameraNormal;

	uint32 captureFileNameCount;
};
