// Copyright (c) Gallant Lab. All Rights Reserved.
// Spectator controller for replaying and rendering MRI experiment demos.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ExperimentFrameCaptureCamera.h"
#include "ExperimentState.h"
#include "ExperimentLoggerComponent.h"
#include "ExperimentSettings.h"

#include "ExperimentSpectatorController.generated.h"

class UExperimentDemoSubsystem;
class AExperimentSubjectState;
class UDemoNetDriver;
class UUserWidget;
class USemSegTagger;

/**
 * Player controller used during demo playback to follow the subject and capture data.
 *
 * During replay, a spectator controller is spawned to track the subject pawn,
 * attach cameras, log per-frame experiment state, and save
 * rendered frames to disk. Subclass this controller to add experiment-specific logging.
 */
UCLASS()
class EXPERIMENT_API AExperimentSpectatorController : public APlayerController
{
	GENERATED_BODY()

public:

	/**
	 * Constructor.
	 * @param objectInitializer  Unreal object initializer forwarded to the parent class.
	 */
	AExperimentSpectatorController(const FObjectInitializer& objectInitializer);

	/** Destructor. */
	~AExperimentSpectatorController();

	/**
	 * All the framewise logic
	 * @param deltaTime  Time elapsed since the last frame in seconds.
	 */
	virtual void Tick(float deltaTime) override;

	/**
	 * Sets the reference pawn to follow, i.e. the player's pawn.
	 * @param newReferencePawn  The pawn to follow and record data from.
	 */
	void SetReferencePawn(APawn* newReferencePawn);

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
	 * Save camera outputs from the current frame.
	 */
	void CaptureCameras();

	/** Are we writing frames from cameras to disk? */
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
	void AddExperimentState(ExperimentState* newState);

	/**
	 * Get the current state at this tick of the replicated player pawn.
	 * @return Pointer to the AExperimentSubjectState, or nullptr if not yet resolved.
	 */
	UFUNCTION(BlueprintPure, Category = "Experiment|HUD")
	AExperimentSubjectState* GetReferencePlayerState() const
	{
		return referencePlayerState;
	}

	/** Delegate fired to update the on-screen text prompt during replay. */
	FSetDisplayTextDelegate SetDisplayTextDelegate;

	/** Delegate fired to change the color of the on-screen text prompt during replay. */
	FSetDisplayTextColorDelegate SetDisplayTextColorDelegate;

	/** Delegate fired to update the displayed points value during replay. */
	FSetDisplayPointsDelegate SetDisplayPointsDelegate;

	/** Delegate fired to update the displayed time-remaining value during replay. */
	FSetTimeRemainingDelegate SetTimeRemainingDelegate;

	/** Binds the widget event that receives the on-screen prompt text.
	 *  Fires immediately with an empty string so the display starts cleared. */
	UFUNCTION(BlueprintCallable, Category = "Experiment|HUD")
	void BindSetDisplayTextDelegate(UPARAM(DisplayName = "Event") FSetDisplayTextDelegate delegate)
	{
		SetDisplayTextDelegate = delegate;
		SetDisplayTextDelegate.ExecuteIfBound(FString());
	}

	/** Binds the widget event that receives the prompt text color. */
	UFUNCTION(BlueprintCallable, Category = "Experiment|HUD")
	void BindSetDisplayTextColorDelegate(UPARAM(DisplayName = "Event") FSetDisplayTextColorDelegate delegate)
	{
		SetDisplayTextColorDelegate = delegate;
	}

	/** Binds the widget event that receives the points value. */
	UFUNCTION(BlueprintCallable, Category = "Experiment|HUD")
	void BindSetDisplayPointsDelegate(UPARAM(DisplayName = "Event") FSetDisplayPointsDelegate delegate)
	{
		SetDisplayPointsDelegate = delegate;
	}

	/** Binds the widget event that receives the time-remaining value. */
	UFUNCTION(BlueprintCallable, Category = "Experiment|HUD")
	void BindSetTimeRemainingDelegate(UPARAM(DisplayName = "Event") FSetTimeRemainingDelegate delegate)
	{
		SetTimeRemainingDelegate = delegate;
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
	 * Returns the current playback time of the replay.
	 * @return Demo time in seconds, or 0 if no demo net driver is active.
	 */
	double GetDemoCurrentTime() const;

	/**
	 * Collects all registered agent components in the world.
	 * Iterates every actor and gathers agent components whose
	 * bRegisterAgentComponent opt-in flag left them registered.
	 * @return Array of registered agent components to visit for state logging.
	 */
	TArray<const UExperimentAgentComponent*> GetAgents() const;

	/**
	 * Returns the UExperimentDemoSubsystem for this session.
	 * @return Pointer to the demo subsystem, or nullptr if not found.
	 */
	UExperimentDemoSubsystem* GetDemoSubsystem() const
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
	 * Spawns and configures the UExperimentLoggerComponent used to collect per-frame state.
	 * Override in subclasses to spawn a specialized logger component.
	 */
	virtual void SpawnLoggerComponent();

	/**
	 * Creates the cameras listed in the settings file. Called once the
	 * reference pawn is known, before the capture folder is applied.
	 * Spawns one camera per description, applies its effect, size, and FOV,
	 * attaches it via AttachCamera(), and registers it with AddCamera().
	 */
	virtual void CreateCameras();

	/**
	 * Attaches a newly created camera. The base implementation attaches to the
	 * reference pawn's root; override to attach to a specific component, such
	 * as the pawn's gameplay camera.
	 * @param camera  The camera to attach.
	 */
	virtual void AttachCamera(AExperimentFrameCaptureCamera* camera);

	/**
	 * Override to fill semanticLabels, semanticTagLabels, and
	 * defaultSemanticLabel. Called right before the level is tagged, when a
	 * created camera uses the SemanticSegmentation effect.
	 */
	virtual void ConfigureSemanticLabels() {}

	/**
	 * Registers a camera with the capture pipeline, so it receives
	 * SetSaveFolder() and SaveNextFrame() calls.
	 * @param camera  The camera to register.
	 */
	void AddCamera(AExperimentFrameCaptureCamera* camera)
	{
		cameras.Add(camera);
	}

protected:

	/** HUD widget class to show during replay. Set it in a subclass constructor
	 *  (or on a Blueprint subclass) to show the same HUD the subject saw. */
	UPROPERTY(EditAnywhere, Category = "Experiment|HUD")
	TSubclassOf<UUserWidget> replayHUDClass = nullptr;

	/** The HUD widget created from replayHUDClass. */
	UPROPERTY()
	UUserWidget* replayHUD = nullptr;

	/** The subject pawn being followed during replay. */
	UPROPERTY()
	APawn* referencePawn = nullptr;

	/** Logger component responsible for per-frame state collection. */
	UPROPERTY()
	UExperimentLoggerComponent* loggerComponent = nullptr;

	/** The player state of the reference subject pawn. */
	UPROPERTY()
	AExperimentSubjectState* referencePlayerState = nullptr;

	/** Array of cameras attached to this spectator. */
	UPROPERTY()
	TArray<AExperimentFrameCaptureCamera*> cameras;

	/** Actor class to stencil label map for semantic segmentation.
	 *  Fill in a subclass before the replay starts. Tagging runs only when a
	 *  created camera uses the SemanticSegmentation effect. */
	UPROPERTY(EditAnywhere, Category = "Experiment|Cameras")
	TMap<TSubclassOf<AActor>, uint8> semanticLabels;

	/** Actor tag to stencil label map for semantic segmentation.
	 *  A tag entry overrides the class map, so it singles out individual
	 *  level actors, such as a sky sphere. */
	UPROPERTY(EditAnywhere, Category = "Experiment|Cameras")
	TMap<FName, uint8> semanticTagLabels;

	/** Semantic label for actors that match no semanticLabels entry.
	 *  0 leaves them untagged, which renders black. */
	UPROPERTY(EditAnywhere, Category = "Experiment|Cameras")
	uint8 defaultSemanticLabel = 0;

	/** Tagger that writes the semantic labels into the custom depth stencil.
	 *  Created only when a semantic segmentation camera exists. */
	UPROPERTY()
	USemSegTagger* semSegTagger = nullptr;

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
	UPROPERTY()
	UDemoNetDriver* demoNetDriver = nullptr;

	/** World settings, used to fix the time step during replay. */
	UPROPERTY()
	AWorldSettings* worldSettings = nullptr;

	/** Buffer of per-frame experiment state snapshots accumulated during playback. Elements are owned and deleted by this controller. */
	TArray<ExperimentState*> frameStates;

	/** Experiment type for this session, used during log serialization. */
	EExperimentType experimentType = EExperimentType::Other;

	/** Zero-based index of the frame currently being rendered. */
	int frameNumber;

	/** Unique ID of the player agent component, used for log identification. */
	uint32 playerAgentID;

	/** Cached pointer to the demo subsystem. */
	UPROPERTY()
	UExperimentDemoSubsystem* demoSubsystem = nullptr;

	/** True when the replay has finished and this controller should be destroyed. */
	bool stopDemo = false;
};
