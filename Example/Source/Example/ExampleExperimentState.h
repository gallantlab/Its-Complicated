// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExperimentState.h"

/**
 *  Per-frame log snapshot for the example experiment.
 *  Records the player entity and the locations of the uncollected trial items,
 *  and marks the frame with whether a trial is running.
 */
class ExampleExperimentState : public ExperimentState
{
public:

	/**
	 * Constructs the snapshot and records the full frame.
	 * Record() runs in this constructor body, after the object is fully
	 * constructed as this class, so the Visit overrides below are the
	 * ones that execute.
	 * @param time           Simulation time in seconds.
	 * @param frame          Zero-based replay frame index.
	 * @param agents         Array of agent components to visit and record.
	 * @param playerState    The subject's replicated player state.
	 * @param itemLocations  World-space locations of the uncollected trial items.
	 */
	ExampleExperimentState(double time, int frame, TArray<const UExperimentAgentComponent*> agents, AExperimentSubjectState* playerState, const TArray<FVector>& itemLocations)
		: ExperimentState(time, frame),
		  itemLocations(itemLocations)
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

protected:

	/**
	 * Appends the InTrial attribute. A trial runs exactly while trial items exist.
	 * @param stream  Output file stream.
	 */
	virtual void WriteFrameAttributes(std::ofstream& stream) override
	{
		stream << Experiment_WRITE_ATTRIBUTE_KV("InTrial", (itemLocations.Num() > 0));
	}

	/**
	 * Writes the base contents, then one Item tag per uncollected trial item.
	 * @param stream  Output file stream.
	 * @param indent  Current indentation level (modified in place).
	 */
	virtual void WriteContents(std::ofstream& stream, int& indent) override
	{
		ExperimentState::WriteContents(stream, indent);

		for (const FVector& location : itemLocations)
		{
			Experiment_WRITE_INDENTS(stream, indent);
			stream << "<Item"
				<< Experiment_WRITE_ATTRIBUTE_KV("X", location.X)
				<< Experiment_WRITE_ATTRIBUTE_KV("Y", location.Y)
				<< Experiment_WRITE_ATTRIBUTE_KV("Z", location.Z)
				<< " />" << std::endl;
		}
	}

	/** World-space locations of the uncollected trial items at this frame. */
	TArray<FVector> itemLocations;
};
