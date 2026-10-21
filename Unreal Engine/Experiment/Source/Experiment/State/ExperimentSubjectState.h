// THis is the core class that records state. Everything you want to know needs to be stored in here.

#pragma once

#include "GameFramework/PlayerState.h"
#include "ExperimentSubjectState.generated.h"

/**
 * Types of on-screen prompts that can be displayed to the subject.
 *
 * Recorded in the experiment log so that analysts can correlate neural data
 * with what was shown on screen at each point in the session.
 */
UENUM(BlueprintType)
enum class EDisplayedPromptType : uint8
{
	/** No prompt is being displayed. */
	None,
	/** A prompt of unspecified or unknown type is being displayed. */
	Unknown,
	/** The "session ended" message is being displayed. */
	SessionEnd,
	/** A "paused" indicator is being displayed. */
	Paused,
};

/**
 * Enumeration of possible display text colors for on-screen prompts and feedback.
 */
UENUM(BlueprintType)
enum class EDisplayTextColor : uint8
{
	/** Display text in white. */
	White	UMETA(DisplayName = "White"),
	/** Display text in red (used for error or warning prompts). */
	Red		UMETA(DisplayName = "Red"),
};

/** Delegate used to set the on-screen display text string. */
DECLARE_DYNAMIC_DELEGATE_OneParam(FSetDisplayTextDelegate, const FString&, Text);

/** Delegate used to set the color of the on-screen display text. */
DECLARE_DYNAMIC_DELEGATE_OneParam(FSetDisplayTextColorDelegate, EDisplayTextColor, Color);

/** Delegate used to update the displayed points value. */
DECLARE_DYNAMIC_DELEGATE_OneParam(FSetDisplayPointsDelegate, int, Points);

/** Delegate used to update the displayed time-remaining value. */
DECLARE_DYNAMIC_DELEGATE_OneParam(FSetTimeRemainingDelegate, int, SecondsRemaining);

/**
 * Replicated player state holding all per-frame data needed for experiment logging and replay.
 *
 * This is the central state object for the MRI experiment framework. All information that
 * must be available to both the live game and the spectator replay controller (e.g. position,
 * TTL state, score) should be stored here as UPROPERTY(Replicated) fields.
 *
 * Access is restricted to AExperimentSubjectController via friend declaration to enforce
 * that state mutations always go through the controller's logic.
 */
UCLASS()
class EXPERIMENT_API AExperimentSubjectState : public APlayerState
{
	GENERATED_BODY()

public:

	/** Initializes all replicated fields to their default values. */
	AExperimentSubjectState();

	/**
	 * Resets the player state to its initial values.
	 * Also calls ResetExperimentState to clear experiment-specific fields.
	 */
	virtual void Reset() override;

	/**
	 * Resets only the experiment-specific state (not base PlayerState fields).
	 * Called at the start of each new run.
	 */
	virtual void ResetExperimentState();

	/**
	 * Copies replicated properties from another player state.
	 * Used during seamless travel to preserve state across level transitions.
	 * @param playerState  Source player state to copy from.
	 */
	virtual void CopyProperties(APlayerState *playerState) override;

	/**
	 * Registers which properties are replicated.
	 * **IMPORTANT** In the implementation, you **MUST** call DOREPLIFETIME for each replicated property, 
	  * or it won't be replicated and you can't read it back out later!
	 * @param outLifetimeProps  Output array to populate with replication rules.
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	/**
	 * Returns the current simulation frame number.
	 * @return Monotonically increasing frame counter.
	 */
	UFUNCTION(BlueprintCallable)
	int64 GetFrameNumber() const
	{
		return FrameNumber;
	}

	/**
	 * Returns the duration of the last simulation step in seconds.
	 * @return Last frame's delta time.
	 */
	UFUNCTION(BlueprintCallable)
	float GetSimulationStepInSeconds() const
	{
		return SimulationStepInSeconds;
	}

	/**
	 * Returns the platform timestamp for the current frame (milliseconds since epoch).
	 * @return Platform timestamp as a 32-bit integer.
	 */
	UFUNCTION(BlueprintCallable)
	int32 GetPlatformTimeStamp() const
	{
		return PlatformTimeStamp;
	}

	/**
	 * Returns the in-game timestamp for the current frame in milliseconds.
	 * @return Game time stamp as a 32-bit integer.
	 */
	UFUNCTION(BlueprintCallable)
	int32 GetGameTimeStamp() const
	{
		return GameTimeStamp;
	}

	/**
	 * Returns whether a The TTL is currently pressed.
	 * @return true if the TTL flag is set.
	 */
	UFUNCTION(BlueprintCallable)
	bool isTTL() const
	{
		return TTL;
	}

	/**
	 * Returns the total number of TTLs received since the start of the experiment.
	 * @return Cumulative TTL count.
	 */
	UFUNCTION(BlueprintCallable)
	int GetTotalTTLs() const
	{
		return TotalTTLs;
	}

	/**
	 * Returns the subject's current world transform (position + rotation + scale).
	 * @return Const reference to the replicated Transform.
	 */
	UFUNCTION(BlueprintCallable)
	const FTransform &GetSubjectTransform() const
	{
		return Transform;
	}

	/**
	 * Returns the subject's current world-space location.
	 * @return World position vector extracted from Transform.
	 */
	UFUNCTION(BlueprintCallable)
	FVector GetLocation() const
	{
		return Transform.GetLocation();
	}

	/**
	 * Returns the subject's current forward orientation vector.
	 * @return Unit vector pointing in the subject's facing direction.
	 */
	UFUNCTION(BlueprintCallable)
	FVector GetOrientation() const
	{
		return Transform.GetRotation().GetForwardVector();
	}

	/**
	 * Returns elapsed seconds between demo start and the first TTL of this run.
	 * @return Time in seconds from demo start to first TTL.
	 */
	UFUNCTION(BlueprintCallable)
	float GetSecondsToStartOfRun() const
	{
		return secondsToStartOfRun;
	}

	/**
	 * Returns the subject's accumulated score for this run.
	 * @return Current integer points total.
	 */
	UFUNCTION(BlueprintCallable)
	int GetPoints() const
	{
		return Points;
	}

	/**
	 * Returns the type of on-screen prompt currently being displayed.
	 * @return EDisplayedPromptType enum value.
	 */
	UFUNCTION(BlueprintCallable)
	EDisplayedPromptType GetDisplayedPromptType() const
	{
		return displayedPromptType;
	}

protected:

	/**
	 * Sets the subject's accumulated points.
	 * Protected so that subclasses can write the field; friendship with
	 * AExperimentSubjectController does not extend to subclasses.
	 * @param newPoints  New points total.
	 */
	void SetPoints(int newPoints)
	{
		Points = newPoints;
	}

	/**
	 * Sets the type of prompt recorded as currently displayed.
	 * Protected so that subclasses can write the field; friendship with
	 * AExperimentSubjectController does not extend to subclasses.
	 * @param type  Prompt type to record.
	 */
	void SetDisplayedPromptType(EDisplayedPromptType type)
	{
		displayedPromptType = type;
	}

private:

	friend class AExperimentSubjectController;

	/** Monotonically increasing simulation frame counter. */
	UPROPERTY(VisibleAnywhere)
	int64 FrameNumber;

	/** Duration of the last simulation step in seconds. */
	UPROPERTY(VisibleAnywhere)
	float SimulationStepInSeconds;

	/** Platform clock timestamp for the current frame (milliseconds since epoch). */
	UPROPERTY(VisibleAnywhere)
	int32 PlatformTimeStamp;

	/** In-game time stamp for the current frame in milliseconds. */
	UPROPERTY(VisibleAnywhere)
	int32 GameTimeStamp = 0.0f;

	/** Subject's world-space transform, replicated to all clients. */
	UPROPERTY(Replicated)
	FTransform Transform;

	/** True while a TTL input is down on this frame. */
	UPROPERTY(Replicated)
	bool TTL = false;

	/** Total number of TTLs received since the start of the experiment */
	UPROPERTY(Replicated)
	int TotalTTLs = 0;

	/** Seconds elapsed from demo start to the first TTL of this run */
	UPROPERTY(Replicated)
	float secondsToStartOfRun = 0.0;

	/** Subject's accumulated score for this run */
	UPROPERTY(Replicated)
	int Points = 0;

	/** Type of on-screen prompt currently displayed to the subject*/
	UPROPERTY(Replicated)
	EDisplayedPromptType displayedPromptType = EDisplayedPromptType::None;

	/**
	 * Updates FrameNumber, PlatformTimeStamp, GameTimeStamp, and SimulationStepInSeconds.
	 * Called by AExperimentSubjectController each tick.
	 * @param deltaSeconds  Time elapsed since the last frame in seconds.
	 */
	void UpdateTimeStamp(float deltaSeconds);
};
