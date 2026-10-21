// Experiment Plugin - Frame Capture Camera

#include "ExperimentFrameCaptureCamera.h"
#include "Experiment.h"
#include "Components/DrawFrustumComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Materials/Material.h"
#include "Misc/FileHelper.h"

static const TCHAR* SEMANTIC_SEGMENTATION_MATERIAL_PATH =
	TEXT("/Experiment/PostProcessingMaterials/SemSegPostProcessMaterial.SemSegPostProcessMaterial");

AExperimentFrameCaptureCamera::AExperimentFrameCaptureCamera(const FObjectInitializer& objectInitializer)
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
	postProcessEffect = EExperimentPostProcessEffect::SceneFinal;
	captureFileNameCount = 0;

	postProcessDepth = nullptr;
	postProcessSemanticSegmentation = nullptr;
	postProcessWorldNormal = nullptr;
	postProcessCameraNormal = nullptr;
}

void AExperimentFrameCaptureCamera::PostActorCreated()
{
	Super::PostActorCreated();
	InitCaptureRender();
}

void AExperimentFrameCaptureCamera::BeginPlay()
{
	Super::BeginPlay();
}

void AExperimentFrameCaptureCamera::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	Super::EndPlay(endPlayReason);
}

void AExperimentFrameCaptureCamera::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);
}

void AExperimentFrameCaptureCamera::InitCaptureRender()
{
	captureRenderTarget = NewObject<UTextureRenderTarget2D>();
	captureRenderTarget->InitCustomFormat(sizeX, sizeY, PF_B8G8R8A8, false);

	captureComponent2D->TextureTarget = captureRenderTarget;
	captureComponent2D->CaptureSource = SCS_FinalColorLDR;
}

uint32 AExperimentFrameCaptureCamera::GetImageSizeX() const
{
	return sizeX;
}

uint32 AExperimentFrameCaptureCamera::GetImageSizeY() const
{
	return sizeY;
}

EExperimentPostProcessEffect AExperimentFrameCaptureCamera::GetPostProcessEffect() const
{
	return postProcessEffect;
}

float AExperimentFrameCaptureCamera::GetFOVAngle() const
{
	return captureComponent2D->FOVAngle;
}

void AExperimentFrameCaptureCamera::SetImageSize(uint32 newSizeX, uint32 newSizeY)
{
	sizeX = newSizeX;
	sizeY = newSizeY;

	if (captureRenderTarget)
	{
		captureRenderTarget->InitCustomFormat(sizeX, sizeY, PF_B8G8R8A8, false);
	}
}

void AExperimentFrameCaptureCamera::SetPostProcessEffect(EExperimentPostProcessEffect effect)
{
	postProcessEffect = effect;
	ApplyPostProcessEffect();
}

void AExperimentFrameCaptureCamera::ApplyPostProcessEffect()
{
	captureComponent2D->PostProcessSettings.WeightedBlendables.Array.Empty();

	switch (postProcessEffect)
	{
		case EExperimentPostProcessEffect::SemanticSegmentation:
			// Loaded lazily; a constructor-time load can deadlock the async loader.
			if (!postProcessSemanticSegmentation)
				postProcessSemanticSegmentation = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, SEMANTIC_SEGMENTATION_MATERIAL_PATH));
			if (postProcessSemanticSegmentation)
				captureComponent2D->PostProcessSettings.AddBlendable(postProcessSemanticSegmentation, 1.0f);
			else
				UE_LOG(LogExperiment, Warning, TEXT("Semantic segmentation material not found at %s"), SEMANTIC_SEGMENTATION_MATERIAL_PATH);
			break;
		default:
			break;
	}
}

void AExperimentFrameCaptureCamera::SetFOVAngle(float angle)
{
	captureComponent2D->FOVAngle = angle;
}

void AExperimentFrameCaptureCamera::SetTargetGamma(float gamma)
{
	if (captureRenderTarget)
	{
		captureRenderTarget->TargetGamma = gamma;
	}
}

bool AExperimentFrameCaptureCamera::ReadPixels(TArray<FColor>& bitMap) const
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

bool AExperimentFrameCaptureCamera::SaveCaptureToDisk(const FString& filePath)
{
	if (!captureRenderTarget)
		return false;

	TArray<FColor> bitMap;
	if (!ReadPixels(bitMap))
		return false;

	// Post-process materials write alpha 0; force opaque pixels so the PNG is viewable.
	for (FColor& pixel : bitMap)
		pixel.A = 255;

	IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> imageWrapper = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	imageWrapper->SetRaw(bitMap.GetData(), bitMap.Num() * sizeof(FColor), sizeX, sizeY, ERGBFormat::BGRA, 8);

	TArray64<uint8> pngData = imageWrapper->GetCompressed(100);
	return FFileHelper::SaveArrayToFile(pngData, *filePath);
}

bool AExperimentFrameCaptureCamera::SaveNextFrame()
{
	FString path = FPaths::Combine(SaveToFolder, FString::Printf(TEXT("capture_%06d.png"), captureFileNameCount++));
	return SaveCaptureToDisk(path);
}

void AExperimentFrameCaptureCamera::SetSaveFolder(const FString& folder)
{
	SaveToFolder = folder;
}

void AExperimentFrameCaptureCamera::AttachToActor(AActor* targetActor)
{
	if (targetActor)
	{
		AActor::AttachToActor(targetActor, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}
