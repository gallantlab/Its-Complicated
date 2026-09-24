// MRI Experiment Plugin - Base Logger Component

#include "MRILoggerComponent.h"
#include "Demo/MRISpectatorController.h"

UMRILoggerComponent::UMRILoggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	UE_LOG(LogMRI, Log, TEXT("MRILoggerComponent spawned"));
}

void UMRILoggerComponent::SetParentController(AMRISpectatorController* parent)
{
	spectatorController = parent;
}

void UMRILoggerComponent::LogFrame()
{
	if (!playerState)
		playerState = spectatorController->GetReferencePlayerState();

	MRIExperimentState* state = new MRIExperimentState(
		spectatorController->GetDemoCurrentTime(),
		spectatorController->GetFrameNumber());
	state->Record(spectatorController->GetAgents(), playerState);
	spectatorController->AddExperimentState(state);
}
