// Copyright (c) Gallant Lab. All Rights Reserved.

#pragma once

#include "Experiment.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ExperimentSubjectState.h"

#include "ExperimentSubjectController.generated.h"

class UInputAction;
class UExperimentDemoSubsystem;
class UExperimentSettings;
class UExperimentSettingsSubsystem;

/** Empty string constant used as a default value for display text */
const FString EMPTY_STRING = FString("");

// The display text color enum and the four display delegate types are declared
// in ExperimentSubjectState.h, shared with the replay spectator controller.

/**
 * Base controller for subjects in MRI experiments.
 * Handles TTL pulse tracking, experiment lifecycle, demo recording management,
 * eyetracking calibration triggers, and display text for prompts and feedback.
 */
UCLASS(Abstract)
class EXPERIMENT_API AExperimentSubjectController : public APlayerController
{
	GENERATED_BODY()

public:

	/**
	 * Constructor
	 * @param ObjectInitializer  Unreal object initializer forwarded to the parent class.
	 */
	AExperimentSubjectController(const FObjectInitializer& ObjectInitializer);

	/**
	 * Called when the game starts. Reads settings from ExperimentSettings, configures
	 * auto-eyetracking, auto-demo-recording, and auto-stop-demo options, and
	 * applies the display resolution if it has not been set yet.
	 */
	virtual void BeginPlay() override;

	/**
	 * Called every frame. Updates the demo recording state, runs ExperimentTick
	 * if enough TTLs have been received, handles display text countdown, and
	 * auto-stops demo recording if no TTL has arrived within the configured limit.
	 * @param DeltaTime  Time elapsed since the last frame in seconds.
	 */
	virtual void Tick(float DeltaTime) override;

	/**
	 * Called when this controller starts to possess a pawn. Caches a typed reference to
	 * the AExperimentSubjectState and configures the net update frequency for replication.
	 * @param pawn  The pawn being possessed.
	 */
	virtual void OnPossess(APawn* pawn) override;

	/**
	 * Marks eyetracking as ended (state 2). Call this from Blueprint when the
	 * eyetracking calibration sequence has completed.
	 * This is code left from when we implemented a native eyetracking calibration sequence.
	 * Not used if you are using Eyelink and/or the GameMonitor.
	 */
	UFUNCTION(BlueprintCallable)
	void SetEyetrackingEnded()
	{
		eyetrackingState = 2;
	}

	/**
	 * Returns true if the demo recording has finihed but a new one has not yet started.
	 * Useful for end-of-run stuff
	 * @return true on the first tick after demo recording stops.
	 */
	bool IsDemoEnded();

	/**
	 * Pure virtual override point for experiment logic. The core time-based/sequential experiment
	 * logic should be implemented her eby the subclasses.
	 * Called each tick once the required number of TTL pulses have been received.
	 * Subclasses must implement this to drive their experiment state machine.
	 * @param deltaTime  Time elapsed since the last frame in seconds.
	 */
	virtual void ExperimentTick(float deltaTime) {}

	/**
	 * Called on the on the TTL keydown event.
	 * Sets the TTL flag in subject state, increments the TTL counter, optionally
	 * auto-starts demo recording on the first TTL, and resets the no-TTL timer.
	 */
	virtual void TTLdown();

	/**
	 * Called on the on the TTL keyup event. Unsets the TTL flag in subject state.
	 */
	virtual void TTLup();

	/**
	 * Returns whether the TTL key is currently down.
	 * @return true if the TTL flag is set in the subject state.
	 */
	bool isTTL() const;

	/** Delegate fired to update the on-screen text prompt. */
	FSetDisplayTextDelegate SetDisplayTextDelegate;

	/** Delegate fired to change the color of the on-screen text prompt. */
	FSetDisplayTextColorDelegate SetDisplayTextColorDelegate;

	/** Delegate fired to update the displayed points value. */
	FSetDisplayPointsDelegate SetDisplayPointsDelegate;

	/** Delegate fired to update the displayed time-remaining value. */
	FSetTimeRemainingDelegate SetTimeRemainingDelegate;

	/** Binds the widget event that receives the on-screen prompt text.
	 *  Fires immediately with an empty string so the display starts cleared. */
	UFUNCTION(BlueprintCallable, Category = "Experiment|HUD")
	void BindSetDisplayTextDelegate(UPARAM(DisplayName = "Event") FSetDisplayTextDelegate delegate)
	{
		SetDisplayTextDelegate = delegate;
		SetDisplayTextDelegate.ExecuteIfBound(EMPTY_STRING);
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

protected:

	/** Seconds remaining before the current display text is automatically cleared. */
	double displayTextTimeRemaining = 0;

	/**
	 * Increment the total number of points
	 * @param increment  Amount to add to the running points total (default: 1).
	 */
	virtual void UpdatePoints(int increment = 1);

	/**
	 * Sets the on-screen display text, its color, and how long it should remain visible.
	 * This base class manages the countdown timer; subclasses do not need to clear the text.
	 * @param text                   Text string to display on screen.
	 * @param duration               Time in seconds to show the text; use -1 to display indefinitely.
	 * @param displayedPromptType    Semantic type of the prompt (for logging purposes).
	 * @param color                  Color in which to render the text.
	 */
	void SetDisplayText(const FString& text, double duration = 2.0,
						EDisplayedPromptType displayedPromptType = EDisplayedPromptType::Unknown,
						EDisplayTextColor color = EDisplayTextColor::White);

	/**
	 * Overload to let you put in different things without specifying all the parameters.
	 * Delegates to the primary overload.
	 * @param text                   Text string to display on screen.
	 * @param displayedPromptType    Semantic type of the prompt (for logging purposes).
	 * @param duration               Time in seconds to show the text; use -1 to display indefinitely.
	 * @param color                  Color in which to render the text.
	 */
	void SetDisplayText(const FString& text,
						EDisplayedPromptType displayedPromptType = EDisplayedPromptType::Unknown,
						double duration = 2.0,
						EDisplayTextColor color = EDisplayTextColor::White);

	/**
	 * Clears the on-screen display text and resets the displayed prompt type to None.
	 * Also calls OnEndDisplayText to notify subclasses.
	 */
	void ClearDisplayText();

	/**
	 * Called when the display text countdown expires and the text is cleared.
	 * Override in subclasses to react to the end of a text display period.
	 */
	virtual void OnEndDisplayText();

	/**
	 * Resets all experiment-level state for a new run (demo state, time counters,
	 * eyetracking state, TTL countdown, and subject state).
	 */
	virtual void ResetExperimentState();

	/**
	 * Binds TTL and all input actions to the corresponding handler methods.
	 */
	virtual void SetupInputComponent() override;

	/** Optional Enhanced Input action for the TTL pulse. When assigned and the
	 * project uses an Enhanced Input component, TTL binds through it. When unset,
	 * the legacy "TTL" action name from the project's input settings is bound. */
	UPROPERTY(EditAnywhere, Category = "Experiment|Input")
	UInputAction* TTLAction = nullptr;

	/**
	 * Records the timestamp (in seconds since demo start) at which the first TTL
	 * of this run was received.
	 * @param seconds  Elapsed seconds from demo start to the first TTL.
	 */
	UFUNCTION(BlueprintCallable)
	void SetSecondsToStartOfRun(float seconds);

	/** True if no TTL has been received yet for the current run. */
	bool isFirstTTLInRun = true;

	/** Resets the subject's accumulated points to zero. */
	void ResetPoints()
	{
		subjectState->Points = 0;
	}

	/** Seconds elapsed since the demo recording started for this run. */
	double timeSinceDemoStart = 0.0;

	/** Seconds elapsed between demo start and the first TTL of the run. */
	double secondsToFirstTTL = 0.0;

	/**
	 * Attempts to locate the UExperimentDemoSubsystem on the current game instance.
	 * Logs an error if the subsystem cannot be found.
	 * @return true if the subsystem was found and cached in demoSubsystem.
	 */
	bool FindDemoSubsystem();

	/** Cached pointer to the demo recording subsystem. May be null before BeginPlay. */
	UPROPERTY()
	UExperimentDemoSubsystem* demoSubsystem = nullptr;

	/** Cached pointer to the settings subsystem. Set during BeginPlay. */
	UPROPERTY()
	UExperimentSettingsSubsystem* settingsSubsystem = nullptr;

	/**
	 * Returns the cached demo subsystem, re-querying it if the cache is stale.
	 * @return Pointer to the UExperimentDemoSubsystem.
	 */
	UExperimentDemoSubsystem* GetDemoSubsystem();

	/** If true, eyetracking calibration is triggered automatically at the start of each run. 
	 * This is  left from when we implemented a native eyetracking calibration sequence.
	 * Not used if you are using Eyelink and/or the GameMonitor.
	 */
	bool autoEyetrack = false;
	/** Current eyetracking calibration state: 0 = not started, 1 = started, 2 = ended. 
	 * This is left from when we implemented a native eyetracking calibration sequence.
	 * Not used if you are using Eyelink and/or the GameMonitor.
	 */
	int eyetrackingState = 0;

	/** Random number generator for experiment-level randomization. */
	FRandomStream randomStream;

	/** Pointer to the subject's replicated player state. Set during Possess. */
	UPROPERTY()
	AExperimentSubjectState* subjectState = nullptr;

private:

	/** Demo recording state from the previous tick, used to detect state transitions. */
	int lastDemoState = 0;
	/** Demo recording state from the current tick. */
	int currentDemoState = 0;

	/**
	 * Queries the demo subsystem for the current recording state and caches it.
	 * Called every tick before experiment logic runs.
	 */
	void UpdateDemoState();

	/** Number of TTLs required before experiment logic begins. */
	int TTLsToExperimentStart = 5;

	/** If true, demo recording starts automatically on the first TTL of a run. */
	bool autoTriggerDemo = false;
	/** If true, demo recording stops automatically after secondsWithoutTTL exceeds autoStopDemoLimit. */
	bool bAutoStopDemo = false;
	/** Threshold in seconds without a TTL before auto-stopping demo recording. */
	float autoStopDemoLimit = 0;
	/** Seconds elapsed since the last TTL was received. */
	float secondsWithoutTTL = 0.0;

	// Used for auto render all; wait n seconds into game init before render
	// because calling demoSubsystem->FindReplays() to autotrigger rendering only works once per launch
	// and on the second trigger, no spectator controller is spawned
	/** If true, automatically starts rendering all available replays. */
	bool autoRender = false;
	/** Seconds remaining before the next automatic render is triggered. */
	double secondsUntilRenderNext = 10;
};
