// MRI Experiment Plugin - Base Agent Component

#pragma once

#include "Components/SceneComponent.h"
#include "Agent/MRIAgentComponentVisitor.h"
#include "MRIAgentComponent.generated.h"

/**
 * Abstract base component for all MRI experiment agents (subjects, NPCs, etc.).
 *
 * Attach subclasses of this component to any Actor that should be tracked and
 * visited by the logging system. During BeginPlay the component registers
 * itself with the agent registry (if bRegisterAgentComponent is true), and
 * during EndPlay it deregisters itself automatically.
 *
 * Use AcceptVisitor to let a visitor (e.g. MRIExperimentState) read out the
 * current state information for this agent each frame.
 */
UCLASS(Abstract)
class MRIEXPERIMENT_API UMRIAgentComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	/**
	 * Constructs the component. Disables per-tick updates since agents are
	 * polled on demand by the logging system rather than ticked independently.
	 * @param ObjectInitializer  Unreal object initializer forwarded to the parent class.
	 */
	UMRIAgentComponent(const FObjectInitializer &ObjectInitializer);

	/**
	 * Returns a unique identifier for this component instance derived from its pointer hash.
	 * @return Hashed pointer value used as a stable per-instance ID.
	 */
	uint32 GetId() const
	{
		return GetTypeHash(this);
	}

	/**
	 * Accepts a visitor and calls the visitor's Visit method with this component.
	 *
	 * Override in subclasses to call the appropriate typed Visit overload so
	 * that visitors can distinguish between different agent component types.
	 * @param Visitor  The visitor object that will read state from this component.
	 */
	virtual void AcceptVisitor(IMRIAgentComponentVisitor &Visitor);

protected:

	/**
	 * Called when the game starts. Sets bAgentComponentIsRegistered based on
	 * bRegisterAgentComponent so the logging system knows to include this agent.
	 */
	void BeginPlay() override;

	/**
	 * Called when the actor is removed from the world. Clears bAgentComponentIsRegistered
	 * so the logging system stops tracking this agent.
	 * @param EndPlayReason  Reason the actor's EndPlay was triggered.
	 */
	void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** If true, this component will register itself with the agent registry on BeginPlay. */
	UPROPERTY(EditAnywhere)
	bool bRegisterAgentComponent = true;

	/** Indicates whether this component is currently registered with the agent registry. Read-only in editor. */
	UPROPERTY(VisibleAnywhere, AdvancedDisplay)
	bool bAgentComponentIsRegistered = false;
};
