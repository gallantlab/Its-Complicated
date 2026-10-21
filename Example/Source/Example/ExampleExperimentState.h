// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExperimentState.h"

/**
 *  Per-frame log snapshot for the example experiment.
 *  Records the player entity. Add a Visit override here for each agent
 *  component type your experiment tracks, and override WriteContents
 *  to append custom XML fields.
 */
class ExampleExperimentState : public ExperimentState
{
public:

	/**
	 * Constructs the snapshot and records the full frame.
	 * Record() runs in this constructor body, after the object is fully
	 * constructed as this class, so the Visit overrides below are the
	 * ones that execute.
	 * @param time         Simulation time in seconds.
	 * @param frame        Zero-based replay frame index.
	 * @param agents       Array of agent components to visit and record.
	 * @param playerState  The subject's replicated player state.
	 */
	ExampleExperimentState(double time, int frame, TArray<const UExperimentAgentComponent*> agents, AExperimentSubjectState* playerState)
		: ExperimentState(time, frame)
	{
		Record(agents, playerState);
	}

	/**
	 * Records the subject pawn's agent component as the player entity.
	 * @param agent  The player agent component being visited.
	 */
	virtual void Visit(const UExperimentPlayerAgentComponent& agent) override
	{
		thisType = EEntityType::Player;
	}
};
