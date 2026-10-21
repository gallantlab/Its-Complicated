// Copyright Epic Games, Inc. All Rights Reserved.

#include "ExampleSpectatorController.h"

#include "ExampleCharacter.h"
#include "ExampleLoggerComponent.h"
#include "ExperimentDemoSubsystem.h"
#include "ExperimentFrameCaptureCamera.h"

#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

AExampleSpectatorController::AExampleSpectatorController(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
}

void AExampleSpectatorController::BeginPlay()
{
	Super::BeginPlay();

	// Show the gameplay HUD during replay. Loaded here rather than in the
	// constructor: GameHUD references this class, and a constructor-time load
	// re-enters the loader on startup and deadlocks it.
	if (!replayHUDClass)
		replayHUDClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/Game/Blueprints/Game/GameHUD.GameHUD_C"));
}

void AExampleSpectatorController::ConfigureSemanticLabels()
{
	// The character and the trial boxes get their own labels; the sky sphere is
	// tagged "Sky" in the level; everything else gets the default label.
	semanticLabels.Add(AExampleCharacter::StaticClass(), 1);
	UClass* trialItemClass = StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/Game/Blueprints/BP_TrialItem.BP_TrialItem_C"));
	if (trialItemClass)
		semanticLabels.Add(trialItemClass, 2);
	semanticTagLabels.Add(FName(TEXT("Sky")), 4);
	defaultSemanticLabel = 3;
}

void AExampleSpectatorController::SpawnLoggerComponent()
{
	UGameInstance* gameInstance = GetWorld()->GetGameInstance();
	demoSubsystem = gameInstance->GetSubsystem<UExperimentDemoSubsystem>();

	if (demoSubsystem)
		experimentType = demoSubsystem->GetExperimentType();
	else
		experimentType = EExperimentType::Other;

	loggerComponent = NewObject<UExampleLoggerComponent>(this);
	loggerComponent->SetParentController(this);
}

void AExampleSpectatorController::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	if (demoSubsystem && demoSubsystem->IsPlayingReplay())
		ReconstructHUD(deltaTime);
}

TArray<AActor*> AExampleSpectatorController::GetTrialItems() const
{
	TArray<AActor*> items;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(TEXT("TrialItem")), items);
	return items;
}

void AExampleSpectatorController::ReconstructHUD(float deltaTime)
{
	const int itemCount = GetTrialItems().Num();

	if (itemCount > observedItemCount)
	{
		// Boxes appeared: a trial started
		trialItemTotal = itemCount;
		trialStartDemoTime = GetDemoCurrentTime();
		lastReportedTrialSecond = -1;
		ShowDisplayText(FString::Printf(TEXT("Collect %d items"), trialItemTotal));
	}
	else if (itemCount < observedItemCount)
	{
		if (itemCount == 0)
			ShowDisplayText(FString(TEXT("All Collected")));
		else
			ShowDisplayText(FString::Printf(TEXT("Collected %d/%d"), trialItemTotal - itemCount, trialItemTotal));
	}
	observedItemCount = itemCount;

	// The timer runs while the trial's boxes exist
	if (itemCount > 0)
	{
		const int second = (int)(GetDemoCurrentTime() - trialStartDemoTime);
		if (second != lastReportedTrialSecond)
		{
			lastReportedTrialSecond = second;
			SetTimeRemainingDelegate.ExecuteIfBound(second);
		}
	}

	if (referencePlayerState && referencePlayerState->GetPoints() != lastObservedPoints)
	{
		lastObservedPoints = referencePlayerState->GetPoints();
		SetDisplayPointsDelegate.ExecuteIfBound(lastObservedPoints);
	}

	if (displayTextSecondsRemaining > 0)
	{
		displayTextSecondsRemaining -= deltaTime;
		if (displayTextSecondsRemaining <= 0)
			SetDisplayTextDelegate.ExecuteIfBound(FString());
	}
}

void AExampleSpectatorController::ShowDisplayText(const FString& text)
{
	SetDisplayTextDelegate.ExecuteIfBound(text);
	displayTextSecondsRemaining = 2.0;
}

bool AExampleSpectatorController::GetDirectionToNearestItem(float& angleDegrees) const
{
	angleDegrees = 0.0f;
	if (!referencePawn)
		return false;

	const TArray<AActor*> items = GetTrialItems();
	if (items.Num() == 0)
		return false;

	const FVector pawnLocation = referencePawn->GetActorLocation();
	double nearestDistanceSquared = TNumericLimits<double>::Max();
	FVector nearestLocation = FVector::ZeroVector;

	for (const AActor* item : items)
	{
		const double distanceSquared = FVector::DistSquared(item->GetActorLocation(), pawnLocation);
		if (distanceSquared < nearestDistanceSquared)
		{
			nearestDistanceSquared = distanceSquared;
			nearestLocation = item->GetActorLocation();
		}
	}

	// GetViewRotation() on the replayed character returns the recorded camera rotation
	const double worldYaw = (nearestLocation - pawnLocation).Rotation().Yaw;
	angleDegrees = (float)FRotator::NormalizeAxis(worldYaw - referencePawn->GetViewRotation().Yaw);
	return true;
}

void AExampleSpectatorController::AttachCamera(AExperimentFrameCaptureCamera* camera)
{
	// Attach to the gameplay camera so captured frames match what the subject saw.
	// Fall back to the pawn root when the pawn is not an ExampleCharacter.
	AExampleCharacter* character = Cast<AExampleCharacter>(referencePawn);
	if (character)
		camera->AttachToComponent(character->GetFollowCamera(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	else
		camera->AttachToActor(referencePawn);
}
