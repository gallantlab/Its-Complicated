// Copyright (c) Gallant Lab. All Rights Reserved.
// Spectator controller for replaying and rendering MRI experiment demos.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Sensor/MRIFrameCaptureCamera.h"
#include "Logging/MRIExperimentState.h"
#include "Logging/MRILoggerComponent.h"

#include "MRISpectatorController.generated.h"

class UMRIDemoSubsystem;
class AMRISubjectState;

/**
 * Player controller used during demo playback to follow the subject and capture data.
 *
 * During replay, a spectator controller is spawned to track the subject pawn,
 * attach sensor cameras, log per-frame experiment state, and save
 * rendered frames to disk. Subclass this controller to add experiment-specific logging.
 */
UCLASS()
class MRIEXPERIMENT_API AMRISpectatorController : public APlayerController
{
	GENERATED_BODY()

public:

	/**
	 * Constructor.
	 * @param objectInitializer  Unreal object initializer forwarded to the parent class.
	 */
	AMRISpectatorController(const FObjectInitializer& objectInitializer);

	/** Destructor. */
	~AMRISpectatorController();

	/**
	 * All the framewise logic
	 * @param deltaTime  Time elapsed since the last frame in seconds.
	 */
	virtual void Tick(float deltaTime) override;

	/**
	 * Sets the reference pawn to follow, i.e. the player's pawn.
	 * @param referencePawn  The pawn to follow and record data from.
	 */
	void SetReferencePawn(APawn* referencePawn);

	/**
	 * Called after the actor is created in the world. Locates the demo subsystem
	 * and configures frame-capture settings from it.
	 */
	virtual void PostActorCreated() override;

	/**
	 * Called when the controller enters the playing state. Spawns the logger component
	 * and begins tracking the replaying pawn.
	 */
	virtual void BeginPlayingState() override;

	/**
	 * Search for the subject pawn in all the actors in the world.
	 * @return Pointer to the tagged player pawn, or nullptr if not found.
	 */
	APawn* FindTaggedPlayerPawn();

	/**
	 * Save sensor outputs from the current frame.
	 */
	void CaptureSensors();

	/** Are we writing frames from sensors to disk? */
	bool captureFrames;

	/**
	 * Sets the folder where rendered frames will be written.
	 * @param folder  Absolute path to the output directory.
	 */
	void SetFrameCaptureFolder(const FString& folder);

	/**
	 * Writes all accumulated per-frame experiment state to a log file on disk.
	 * Override to add custom experiment info.
	 */
	virtual void SaveExperimentLog();

	/**
	 * Returns whether the replay has finished and this instance should be cleaned up..
	 * @return true if the replay has ended.
	 */
	bool IsStale() { return stopDemo; }

	/**
	 * Appends a the current state of the experiment from this current tick to log.
	 * @param newState  Heap-allocated state object. Ownership is transferred to this controller.
	 */
	void AddExperimentState(MRIExperimentState* newState);

	/**
	 * Get the current state at this tick of the replicated player pawn.
	 * @return Pointer to the AMRISubjectState, or nullptr if not yet resolved.
	 */
	AMRISubjectState* GetReferencePlayerState() const
	{
		return referencePlayerState;
	}

	/**
	 * Returns the UDemoNetDriver driving the current replay playback.
	 * @return Pointer to the demo net driver, or nullptr if not available.
	 */
	UDemoNetDriver* GetDemoNetDriver() const
	{
		return demoNetDriver;
	}

	/**
	 * Returns the UMRIDemoSubsystem for this session.
	 * @return Pointer to the demo subsystem, or nullptr if not found.
	 */
	UMRIDemoSubsystem* GetDemoSubsystem() const
	{
		return demoSubsystem;
	}

	/**
	 * Returns the current replay frame number.
	 * @return Zero-based index of the frame currently being processed.
	 */
	int GetFrameNumber() const
	{
		return frameNumber;
	}

protected:

	/**
	 * Attempts to set the view target to the replaying subject pawn.
	 */
	void TrySetViewTargetToPlayer();

	/**
	 * Enables or disables fixed time-stepping. When fixed, game time is decoupled from wall time.
	 * @param fix  true to lock the time step.
	 */
	void FixTimeStep(bool fix);

	/**
	 * Captures the state from all registered agents on this current tick,
	 * then appends the snapshot to the internal log buffer.
	 */
	void LogExperimentState();

	/**
	 * Spawns and configures the UMRILoggerComponent used to collect per-frame state.
	 * Override in subclasses to spawn a specialized logger component.
	 */
	virtual void SpawnLoggerComponent();

private:

	/** The subject pawn being followed during replay. */
	APawn* referencePawn = nullptr;

	/** Logger component responsible for per-frame state collection. */
	UMRILoggerComponent* loggerComponent = nullptr;

	/** The player state of the reference subject pawn. */
	AMRISubjectState* referencePlayerState = nullptr;

	/** Array of frame-capture camera sensors attached to this spectator. */
	TArray<AMRIFrameCaptureCamera*>* sensors = nullptr;

	/** True until the reference pawn has been located and locked as the view target. */
	bool needReference = true;

	/** Target frames per second for the replay time step. */
	int fps = 0;

	/** Accumulated replay time in seconds. -1.0 indicates not yet started. */
	double currentTime = -1.0;

	/** Fixed time step duration in seconds (1.0 / fps). */
	double timeStep = 0.0;

	/** Initial delay in seconds before frame capture begins, to allow the scene to settle. */
	double delay = 5.0;

	/** The UDemoNetDriver driving the current replay playback. */
	UDemoNetDriver* demoNetDriver = nullptr;

	/** World settings, used to fix the time step during replay. */
	AWorldSettings* worldSettings = nullptr;

	/** Buffer of per-frame experiment state snapshots accumulated during playback. */
	TArray<MRIExperimentState*>* frameStates = nullptr;

	/** Experiment type for this session, used during log serialization. */
	EExperimentType experimentType = EExperimentType::Other;

	/** Zero-based index of the frame currently being rendered. */
	int frameNumber;

	/** Unique ID of the player agent component, used for log identification. */
	uint32 playerAgentID;

	/** Cached pointer to the demo subsystem. */
	UMRIDemoSubsystem* demoSubsystem = nullptr;

	/** True when the replay has finished and this controller should be destroyed. */
	bool stopDemo = false;
};
