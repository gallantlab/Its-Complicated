// Copyright (c) Gallant Lab. All Rights Reserved.

#pragma once

#include "ExperimentAgentComponent.h"
#include "ExperimentPlayerAgentComponent.generated.h"

/**
 * Agent component marking the subject's (player's) pawn for state logging.
 *
 * Attach to the pawn the subject controls during the experiment. During replay,
 * the logging system visits this component each tick and records the pawn's
 * position and rotation as an EEntityType::Player entity in the XML log.
 */
UCLASS(ClassGroup = (Experiment), meta = (BlueprintSpawnableComponent))
class EXPERIMENT_API UExperimentPlayerAgentComponent : public UExperimentAgentComponent
{
	GENERATED_BODY()

public:

	/**
	 * Constructs the component.
	 * @param ObjectInitializer  Unreal object initializer forwarded to the parent class.
	 */
	UExperimentPlayerAgentComponent(const FObjectInitializer &ObjectInitializer);

	/**
	 * Accepts a visitor and calls the player-typed Visit overload,
	 * so visitors record this entity as EEntityType::Player.
	 * @param Visitor  The visitor object that will read state from this component.
	 */
	virtual void AcceptVisitor(IExperimentAgentComponentVisitor &Visitor) const override;
};
