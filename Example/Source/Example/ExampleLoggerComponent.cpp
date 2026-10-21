// Copyright Epic Games, Inc. All Rights Reserved.

#include "ExampleLoggerComponent.h"

#include "ExampleExperimentState.h"
#include "ExperimentSpectatorController.h"

void UExampleLoggerComponent::LogFrame()
{
	if (!playerState)
		playerState = spectatorController->GetReferencePlayerState();

	spectatorController->AddExperimentState(
		new ExampleExperimentState(
			spectatorController->GetDemoCurrentTime(),
			spectatorController->GetFrameNumber(),
			spectatorController->GetAgents(),
			playerState));
}
