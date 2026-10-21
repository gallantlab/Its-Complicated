// Experiment Plugin - Base Logger Component

#include "ExperimentLoggerComponent.h"
#include "Demo/ExperimentSpectatorController.h"

UExperimentLoggerComponent::UExperimentLoggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	UE_LOG(LogExperiment, Log, TEXT("ExperimentLoggerComponent spawned"));
}

void UExperimentLoggerComponent::SetParentController(AExperimentSpectatorController* parent)
{
	spectatorController = parent;
}

void UExperimentLoggerComponent::LogFrame()
{
	if (!playerState)
		playerState = spectatorController->GetReferencePlayerState();

	ExperimentState* state = new ExperimentState(
		spectatorController->GetDemoCurrentTime(),
		spectatorController->GetFrameNumber());
	state->Record(spectatorController->GetAgents(), playerState);
	spectatorController->AddExperimentState(state);
}
