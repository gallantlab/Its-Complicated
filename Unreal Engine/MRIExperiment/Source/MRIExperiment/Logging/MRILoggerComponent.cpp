// MRI Experiment Plugin - Base Logger Component

#include "MRIExperiment.h"
#include "Logging/MRILoggerComponent.h"
#include "Demo/MRISpectatorController.h"

UMRILoggerComponent::UMRILoggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	UE_LOG(LogTemp, Log, TEXT("MRILoggerComponent spawned"));
}

void UMRILoggerComponent::SetParentController(AMRISpectatorController* parent)
{
	spectatorController = parent;
}

void UMRILoggerComponent::LogFrame()
{
	if (!playerState)
		playerState = spectatorController->GetReferencePlayerState();

	spectatorController->AddExperimentState(
		new MRIExperimentState(
			spectatorController->GetDemoCurrentTime(),
			spectatorController->GetFrameNumber(),
			spectatorController->GetAgents(),
			playerState));
}
