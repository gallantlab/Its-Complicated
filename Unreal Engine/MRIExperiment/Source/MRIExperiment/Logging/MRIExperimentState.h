#pragma once

#include "CoreMinimal.h"
#include "Agent/MRIAgentComponent.h"
#include "Agent/MRIAgentComponentVisitor.h"
#include "State/MRISubjectState.h"

#include <iostream>
#include <fstream>

#ifndef MRI_WRITE_INDENTS
#define MRI_WRITE_INDENTS(out, indents) for (int indent = 0; indent < indents; indent++) out << '\t';
#endif

#ifndef MRI_WRITE_ATTRIBUTE
#define MRI_WRITE_ATTRIBUTE(attribute) '"' << attribute << '"'
#endif

#define MRI_WRITE_ATTRIBUTE_KV(key, value) " " << key << "=\"" << value << '"'

#define MRI_WRITE_SINGLE_TAG(name, content) "<" << name << ">" << content << "</" << name << ">"


/**
 * Tick/frame record of the position, rotation, type, and ID of a single agent entity at one frame.
 *
 * Used as a record inside MRIExperimentState to track all relevant actors during replay.
 */
struct EntityState
{
	/**
	 * Constructs an entity state with all fields populated.
	 * @param position  World-space position of the entity.
	 * @param rotation  World-space rotation of the entity.
	 * @param type      Semantic type of the entity (subject, NPC, etc.).
	 * @param ID        Unique identifier for the entity component instance.
	 */
	EntityState(FVector position, FRotator rotation, EntityType type, uint32 ID)
		: position(position),
		  rotation(rotation),
		  type(type),
		  ID(ID)
	{
	}

	/** World-space position of the entity at the time of capture. */
	FVector position;
	/** World-space rotation of the entity at the time of capture. */
	FRotator rotation;
	/** Semantic entity type (e.g. player, NPC). */
	EntityType type;
	/** Unique hash-based ID of the agent component instance. */
	uint32 ID;
};

/**
 * A single-frame snapshot of all experiment state, collected during demo playback.
 *
 * Each instance records the simulation time, frame number, all agent component positions
 * and rotations, and the subject's replicated player state (TTL, points, beep, etc.).
 * Implements IMRIAgentComponentVisitor so that agent components can push their data in.
 *
 * After collection, call WriteToFile() to serialize the frame as XML-style log output.
 */
class MRIExperimentState : public IMRIAgentComponentVisitor
{
public:

	/**
	 * Constructs a state snapshot for the given simulation time.
	 * @param time  Simulation time in seconds at which this frame was captured.
	 */
	MRIExperimentState(double time);

	/**
	 * Constructs a state snapshot for the given simulation time and frame number.
	 * @param time   Simulation time in seconds.
	 * @param frame  Zero-based replay frame index.
	 */
	MRIExperimentState(double time, int frame);

	/**
	 * Constructs a state snapshot and immediately records states for all given agents.
	 * @param time    Simulation time in seconds.
	 * @param frame   Zero-based replay frame index.
	 * @param agents  Array of agent components to visit and record.
	 */
	MRIExperimentState(double time, int frame, TArray<const UMRIAgentComponent*> agents);

	/**
	 * Constructs a state snapshot with agent states and subject player state.
	 * @param time         Simulation time in seconds.
	 * @param frame        Zero-based replay frame index.
	 * @param agents       Array of agent components to visit and record.
	 * @param playerState  The subject's replicated player state.
	 */
	MRIExperimentState(double time, int frame, TArray<const UMRIAgentComponent*> agents, AMRISubjectState* playerState);

	/**
	 * Visits all agent components in the given array and records their states.
	 * @param agents  Array of agent components to visit.
	 */
	void RecordStates(TArray<const UMRIAgentComponent*> agents);

	/**
	 * Returns the simulation time at which this frame was captured.
	 * @return Time in seconds.
	 */
	double GetFrameTime() const { return time; }

	/**
	 * Returns the rendered frame index for this snapshot.
	 * @return Frame number.
	 */
	int GetFrame() const { return frame; }

	/**
	 * Returns the total number of TTLs recorded at the time of this frame.
	 * @return Cumulative TTL count.
	 */
	int TotalTTLCount() const { return totalTTLs; }

	/**
	 * Returns the type of prompt being displayed at the time of this frame.
	 * @return EDisplayedPromptType enum value.
	 */
	EDisplayedPromptType GetDisplayedPromptType() const { return displayedPromptType; }

	/**
	 * Returns whether a synchronization beep was playing during this frame.
	 * @return true if a beep event was active.
	 */
	bool IsBeep() const { return isBeep; }

	/**
	 * Returns the subject's accumulated points at the time of this frame.
	 * @return Integer points total.
	 */
	int GetPoints() const { return points; }

	/**
	 * Called by UMRIAgentComponent::AcceptVisitor to record this agent's state.
	 * Appends a new EntityState entry for the visited component.
	 * @param agent  The agent component being visited.
	 */
	virtual void Visit(const UMRIAgentComponent& agent) override;

	/**
	 * Returns the EntityState at the given index.
	 * @param index  Zero-based index into the entity states array.
	 * @return Reference to the EntityState at that index.
	 */
	EntityState& operator[](int index) { return entityStates[index]; }

	/**
	 * Returns the number of entity states recorded in this frame.
	 * @return Count of EntityState entries.
	 */
	int Num() const { return entityStates.Num(); }

	/** True if a TTL pulse was active during this frame. */
	bool isTTL;

	/**
	 * Writes the state at this frame out to XML at the given file stream.
	 * @param logFile      Output file stream to write to.
	 * @param indentation  Current indentation level (modified in place).
	 */
	void WriteToFile(std::ofstream& logFile, int& indentation);

	/** Destructor. */
	~MRIExperimentState();

protected:

	/**
	 * Writes the inner XML content for this frame. Override in subclasses to
	 * add experiment-specific fields.
	 * @param stream  Output file stream.
	 * @param indent  Current indentation level (modified in place).
	 */
	virtual void WriteContents(std::ofstream& stream, int& indent);

private:

	/** Simulation time in seconds at which this frame was captured. */
	double time;
	/** Zero-based replay rendering frame index. */
	int frame;
	/** Entity type of the primary tracked actor for this frame. */
	EntityType thisType;
	/** Per-agent position/rotation/type snapshots for this frame. */
	TArray<EntityState> entityStates;

	/** Total TTLs received up to this frame. */
	int totalTTLs = 0;
	/** True if a synchronization beep was active during this frame. */
	bool isBeep;
	/** Subject's accumulated points at the time of this frame. */
	int points = 0;
	/** Type of prompt displayed during this frame. */
	EDisplayedPromptType displayedPromptType;
};
