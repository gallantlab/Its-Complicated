// THis is the core class that records state. Everything you want to know needs to be stored in here.

#pragma once

#include "GameFramework/PlayerState.h"
#include "MRISubjectState.generated.h"

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
 * Replicated player state holding all per-frame data needed for experiment logging and replay.
 *
 * This is the central state object for the MRI experiment framework. All information that
 * must be available to both the live game and the spectator replay controller (e.g. position,
 * TTL state, score, beep events) should be stored here as UPROPERTY(Replicated) fields.
 *
 * Access is restricted to AMRISubjectController via friend declaration to enforce
 * that state mutations always go through the controller's logic.
 */
UCLASS()
class MRIEXPERIMENT_API AMRISubjectState : public APlayerState
{
	GENERATED_BODY()

public:

	/** Initializes all replicated fields to their default values. */
	AMRISubjectState();

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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &outLifetimeProps) const override;

	/**
	 * Returns the current simulation frame number.
	 * @return Monotonically increasing frame counter.
	 */
	UFUNCTION(BlueprintCallable)
	uint64 GetFrameNumber() const
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
	 * Returns whether a synchronization beep is currently playing.
	 * @return true if the PlayBeep flag is set.
	 */
	UFUNCTION(BlueprintCallable)
	bool IsBeep() const
	{
		return PlayBeep;
	}

	/**
	 * Returns the subject's current world transform (position + rotation + scale).
	 * @return Const reference to the replicated Transform.
	 */
	UFUNCTION(BlueprintCallable)
	const FTransform &GetTransform() const
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

private:

	friend class AMRISubjectController;

	/** Monotonically increasing simulation frame counter. */
	UPROPERTY(VisibleAnywhere)
	uint64 FrameNumber;

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

	/** True while the synchronization beep is playing, */
	UPROPERTY(Replicated)
	bool PlayBeep = false;

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
	 * Called by AMRISubjectController each tick.
	 * @param deltaSeconds  Time elapsed since the last frame in seconds.
	 */
	void UpdateTimeStamp(float deltaSeconds);
};
