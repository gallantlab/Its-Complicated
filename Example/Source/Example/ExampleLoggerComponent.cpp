// Copyright Epic Games, Inc. All Rights Reserved.

#include "ExampleLoggerComponent.h"

#include "ExampleExperimentState.h"
#include "ExampleSpectatorController.h"

void UExampleLoggerComponent::LogFrame()
{
	if (!playerState)
		playerState = spectatorController->GetReferencePlayerState();

	TArray<FVector> itemLocations;
	AExampleSpectatorController* exampleSpectator = Cast<AExampleSpectatorController>(spectatorController);
	if (exampleSpectator)
	{
		for (const AActor* item : exampleSpectator->GetTrialItems())
			itemLocations.Add(item->GetActorLocation());
	}

	spectatorController->AddExperimentState(
		new ExampleExperimentState(
			spectatorController->GetDemoCurrentTime(),
			spectatorController->GetFrameNumber(),
			spectatorController->GetAgents(),
			playerState,
			itemLocations));
}
