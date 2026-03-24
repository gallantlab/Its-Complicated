// MRI Experiment Plugin - Frame Capture Camera

#include "MRIExperiment.h"
#include "Sensor/MRIFrameCaptureCamera.h"

#include "Components/DrawFrustumComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"

AMRIFrameCaptureCamera::AMRIFrameCaptureCamera(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	captureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent2D"));
	captureComponent2D->SetupAttachment(RootComponent);
	SetRootComponent(captureComponent2D);

	meshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	meshComponent->SetupAttachment(captureComponent2D);

	drawFrustum = CreateDefaultSubobject<UDrawFrustumComponent>(TEXT("DrawFrustum"));
	drawFrustum->SetupAttachment(captureComponent2D);

	sizeX = 720;
	sizeY = 512;
	postProcessEffect = EMRIPostProcessEffect::SceneFinal;
	captureFileNameCount = 0;

	postProcessDepth = nullptr;
	postProcessSemanticSegmentation = nullptr;
	postProcessWorldNormal = nullptr;
	postProcessCameraNormal = nullptr;
}

void AMRIFrameCaptureCamera::PostActorCreated()
{
	Super::PostActorCreated();
	InitCaptureRender();
}

void AMRIFrameCaptureCamera::BeginPlay()
{
	Super::BeginPlay();
}

void AMRIFrameCaptureCamera::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	Super::EndPlay(endPlayReason);
}

void AMRIFrameCaptureCamera::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);
}

void AMRIFrameCaptureCamera::InitCaptureRender()
{
	captureRenderTarget = NewObject<UTextureRenderTarget2D>();
	captureRenderTarget->InitCustomFormat(sizeX, sizeY, PF_B8G8R8A8, false);
	captureRenderTarget->InitAutoFormat = PF_B8G8R8A8;

	captureComponent2D->TextureTarget = captureRenderTarget;
	captureComponent2D->CaptureSource = SCS_FinalColorLDR;
}

uint32 AMRIFrameCaptureCamera::GetImageSizeX() const
{
	return sizeX;
}

uint32 AMRIFrameCaptureCamera::GetImageSizeY() const
{
	return sizeY;
}

EMRIPostProcessEffect AMRIFrameCaptureCamera::GetPostProcessEffect() const
{
	return postProcessEffect;
}

float AMRIFrameCaptureCamera::GetFOVAngle() const
{
	return captureComponent2D->FOVAngle;
}

void AMRIFrameCaptureCamera::SetImageSize(uint32 newSizeX, uint32 newSizeY)
{
	sizeX = newSizeX;
	sizeY = newSizeY;

	if (captureRenderTarget)
	{
		captureRenderTarget->InitCustomFormat(sizeX, sizeY, PF_B8G8R8A8, false);
	}
}

void AMRIFrameCaptureCamera::SetPostProcessEffect(EMRIPostProcessEffect effect)
{
	postProcessEffect = effect;
}

void AMRIFrameCaptureCamera::SetFOVAngle(float angle)
{
	captureComponent2D->FOVAngle = angle;
}

void AMRIFrameCaptureCamera::SetTargetGamma(float gamma)
{
	if (captureRenderTarget)
	{
		captureRenderTarget->TargetGamma = gamma;
	}
}

bool AMRIFrameCaptureCamera::ReadPixels(TArray<FColor>& bitMap) const
{
	if (!captureRenderTarget)
		return false;

	FTextureRenderTargetResource* renderTargetResource = captureRenderTarget->GameThread_GetRenderTargetResource();
	if (!renderTargetResource)
		return false;

	FReadSurfaceDataFlags readSurfaceDataFlags(RCM_UNorm, CubeFace_MAX);
	renderTargetResource->ReadPixels(bitMap, readSurfaceDataFlags);

	return true;
}

bool AMRIFrameCaptureCamera::SaveCaptureToDisk(const FString& filePath)
{
	if (!captureRenderTarget)
		return false;

	TArray<FColor> bitMap;
	if (!ReadPixels(bitMap))
		return false;

	IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> imageWrapper = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	imageWrapper->SetRaw(bitMap.GetData(), bitMap.GetAllocatedSize(), sizeX, sizeY, ERGBFormat::BGRA, 8);

	const TArray<uint8>& pngData = imageWrapper->GetCompressed(100);
	return FFileHelper::SaveArrayToFile(pngData, *filePath);
}

bool AMRIFrameCaptureCamera::SaveNextFrame()
{
	FString path = FPaths::Combine(SaveToFolder, FString::Printf(TEXT("capture_%06d.png"), captureFileNameCount++));
	return SaveCaptureToDisk(path);
}

void AMRIFrameCaptureCamera::SetSaveFolder(const FString& folder)
{
	SaveToFolder = folder;
}

void AMRIFrameCaptureCamera::AttachToActor(AActor* targetActor)
{
	if (targetActor)
	{
		AActor::AttachToActor(targetActor, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}
