// MRI Experiment Plugin - Base Logger Component

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "State/MRISubjectState.h"
#include "MRILoggerComponent.generated.h"

class AMRISpectatorController;

/**
 * Base actor component that collects per-frame experiment state during demo playback.
 *
 * Attach this component (or a subclass) to the spectator controller. The spectator
 * calls LogFrame() each tick to record the current state of all registered agents
 * and the subject's player state into the experiment log buffer.
 *
 * Subclass this component to add experiment-specific data to each log frame.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MRIEXPERIMENT_API UMRILoggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * Constructs the component. Disables per-tick updates since logging
	 * is driven explicitly by the spectator controller.
	 */
	UMRILoggerComponent();

	/**
	 * Captures the current per-frame experiment state and appends it to the
	 * spectator controller's log buffer. Called by the spectator controller each tick.
	 */
	virtual void LogFrame();

	/**
	 * Sets the owning spectator controller so this component can query its state.
	 * Must be called before LogFrame() is invoked.
	 * @param parent  The AMRISpectatorController that owns this logger.
	 */
	void SetParentController(AMRISpectatorController* parent);

protected:
	/** The spectator controller that owns and drives this logger component. */
	AMRISpectatorController* spectatorController = nullptr;
	/** The subject's replicated player state, resolved lazily from the spectator controller. */
	AMRISubjectState* playerState = nullptr;
};
