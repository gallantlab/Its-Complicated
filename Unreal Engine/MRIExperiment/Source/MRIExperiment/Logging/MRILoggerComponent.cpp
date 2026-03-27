// MRI Experiment Plugin - Base Logger Component

#include "MRIExperiment.h"
#include "Logging/MRILoggerComponent.h"
#include "Demo/MRISpectatorController.h"

/** Constructs the component with ticking disabled; logging is driven by the spectator controller. */
UMRILoggerComponent::UMRILoggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	UE_LOG(LogTemp, Log, TEXT("MRILoggerComponent spawned"));
}

/** Caches the parent spectator controller so LogFrame can query its state each tick. */
void UMRILoggerComponent::SetParentController(AMRISpectatorController* parent)
{
	spectatorController = parent;
}

/**
 * Resolves the player state lazily on first call, then appends a new MRIExperimentState
 * snapshot (including all registered agents) to the spectator controller's log buffer.
 */
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
