// Copyright (c) Gallant Lab. All Rights Reserved.
//
// The demo recording and playback logic in this file was implemented with reference to the
// Unreal Engine community replay system tutorial:
// https://unrealcommunity.wiki/replay-system-tutorial-41kq5b58
// The replay rendering pipeline and spectator controller are original work.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NetworkReplayStreaming.h"

#include "MRIDemoSubsystem.generated.h"

class UMRISettings;

/**
 * Blueprint-accessible struct describing a single replay recording.
 *
 * Populated by the network replay streaming system and exposed to Blueprints
 * so that UI widgets can display a list of available recordings.
 */
USTRUCT(BlueprintType)
struct FS_ReplayInfo
{
	GENERATED_USTRUCT_BODY()

	/** Internal filename used to identify the replay on disk. */
	UPROPERTY(BlueprintReadOnly)
	FString ReplayName;

	/** Human-readable name shown in UI. */
	UPROPERTY(BlueprintReadOnly)
	FString FriendlyName;

	/** Date and time when the replay was recorded. */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	/** Total duration of the replay in milliseconds. */
	UPROPERTY(BlueprintReadOnly)
	int32 LengthInMS;

	/** True if this struct was populated from a real replay; false for default-constructed instances. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsValid;

	/**
	 * Constructs a valid replay info entry with all fields populated.
	 * @param NewName          Internal replay filename.
	 * @param NewFriendlyName  Human-readable replay name.
	 * @param NewTimestamp     Recording timestamp.
	 * @param NewLengthInMS    Duration in milliseconds.
	 */
	FS_ReplayInfo(FString NewName, FString NewFriendlyName, FDateTime NewTimestamp, int32 NewLengthInMS)
	{
		ReplayName = NewName;
		FriendlyName = NewFriendlyName;
		Timestamp = NewTimestamp;
		LengthInMS = NewLengthInMS;
		bIsValid = true;
	}

	/**
	 * Default constructor. Creates an invalid placeholder entry.
	 * bIsValid will be false to indicate no real data is present.
	 */
	FS_ReplayInfo()
	{
		ReplayName = "Replay";
		FriendlyName = "Replay";
		Timestamp = FDateTime::MinValue();
		LengthInMS = 0;
		bIsValid = false;
	}
};

/**
 * Game instance subsystem that manages demo recording, playback, and replay enumeration.
 *
 * Adapted from the driving simulator so that any project can use this functionality
 * without replacing its own game instance class. Attach to any UGameInstance by
 * including the plugin. Provides Blueprint-callable wrappers for all
 * core replay operations, a demo-rendering pipeline for offline frame capture,
 * and OBS integration stubs.
 */
UCLASS()
class MRIEXPERIMENT_API UMRIDemoSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/**
	 * Called by Unreal when the subsystem is created. Loads MRISettings,
	 * determines the experiment type, and prepares the replay streamer.
	 * @param Collection  The subsystem collection managing this subsystem.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Called by Unreal when the subsystem is destroyed. Releases resources
	 * held by the replay streamer.
	 */
	virtual void Deinitialize() override;

	// ------------------------------------------------------------------
	// Settings access
	// ------------------------------------------------------------------

	/**
	 * Returns a reference to the current MRI settings object.
	 * @return Reference to the loaded UMRISettings instance.
	 */
	UMRISettings& GetSettings();

	/**
	 * Returns a const reference to the current MRI settings object.
	 * @return Const reference to the loaded UMRISettings instance.
	 */
	const UMRISettings& GetSettings() const;

	/**
	 * Returns a pointer to the current MRI settings for Blueprint access.
	 * @return Pointer to the UMRISettings object.
	 */
	UFUNCTION(BlueprintCallable, Category = "MRI")
	UMRISettings* GetMRISettings() { return settings; }

	// ------------------------------------------------------------------
	// Replay state queries
	// ------------------------------------------------------------------

	/**
	 * Returns whether a replay is currently being recorded.
	 * @return true if demo recording is active.
	 */
	UFUNCTION(BlueprintPure, Category = "Replays")
	bool IsRecordingReplay()
	{
		return isRecording;
	}

	/**
	 * Returns whether a replay is currently being played back.
	 * @return true if replay playback is active.
	 */
	bool IsPlayingReplay()
	{
		return isReplay;
	}

	// ------------------------------------------------------------------
	// Player pawn helpers
	// ------------------------------------------------------------------

	/**
	 * Returns the player pawn, using tag-based lookup or name-based lookup as appropriate.
	 * @return Pointer to the player's APawn, or nullptr if not found.
	 */
	APawn* GetPlayerPawn();

	/**
	 * Finds the player pawn by searching for an actor tagged with MRIPlayerTag.
	 * @return Pointer to the tagged player APawn, or nullptr if not found.
	 */
	APawn* GetPlayerPawnByTag();

	/**
	 * Finds the player pawn by searching for an actor whose name matches the stored player pawn name.
	 * @return Pointer to the named player APawn, or nullptr if not found.
	 */
	APawn* GetPlayerPawnByName();

	// ------------------------------------------------------------------
	// Core replay operations (delegate to UGameInstance)
	// ------------------------------------------------------------------

	/**
	 * Plays the demo with the given name.
	 * @param replayName  Internal filename of the replay to play.
	 */
	void PlayReplay(const FString& replayName);

	/**
	 * Begins recording a new demo.
	 * @param name          Internal filename to use for the recording.
	 * @param friendlyName  Human-readable name shown in UI.
	 */
	void StartRecordingReplay(const FString& name, const FString& friendlyName);

	/**
	 * Stops the currently active demo recording.
	 */
	void StopRecordingReplay();

	// ------------------------------------------------------------------
	// Blueprint wrappers
	// ------------------------------------------------------------------

	/**
	 * Blueprint-callable wrapper to begin recording a demo.
	 * @param ReplayName    Internal filename for the new recording.
	 * @param FriendlyName  Human-readable name shown in UI.
	 */
	UFUNCTION(BlueprintCallable, Category = "Replays")
	void StartRecordingReplayFromBP(FString ReplayName, FString FriendlyName);

	/**
	 * Blueprint-callable wrapper to stop the currently active demo recording.
	 */
	UFUNCTION(BlueprintCallable, Category = "Replays")
	void StopRecordingReplayFromBP();

	/**
	 * Blueprint-callable wrapper to begin playing back a demo.
	 * @param ReplayName  Internal filename of the demo to play back.
	 */
	UFUNCTION(BlueprintCallable, Category = "Replays")
	void PlayReplayFromBP(FString ReplayName);

	/**
	 * Asynchronously enumerates all available demo files.
	 * When complete, calls BP_OnFindReplaysComplete with the results.
	 */
	UFUNCTION(BlueprintCallable, Category = "Replays")
	void FindReplays();

	/**
	 * Renames an existing replay.
	 * @param ReplayName             Internal filename of the replay to rename.
	 * @param NewFriendlyReplayName  New human-readable name for the replay.
	 */
	UFUNCTION(BlueprintCallable, Category = "Replays")
	void RenameReplay(const FString& ReplayName, const FString& NewFriendlyReplayName);

	/**
	 * Deletes an existing replay from disk.
	 * @param ReplayName  Internal filename of the replay to delete.
	 */
	UFUNCTION(BlueprintCallable, Category = "Replays")
	void DeleteReplay(const FString& ReplayName);

	// ------------------------------------------------------------------
	// Demo rendering
	// ------------------------------------------------------------------

	/**
	 * Plays back a demo and renders each frame to disk using a spectator controller.
	 * Sets up the capture folder, FPS, and demo name, then calls PlayReplay.
	 * @param demoName  Internal filename of the demo to render.
	 * @param fps       Frame rate at which to render the demo.
	 */
	UFUNCTION(BlueprintCallable, Category = "Replays")
	void DemoRenderFrames(const FString& demoName, int fps);

	/**
	 * Returns whether the spectator controller should capture frames during playback.
	 * @return true if frame capture is enabled.
	 */
	bool CaptureFrames() const
	{
		return spectatorToCaptureFrames;
	}

	/**
	 * Returns folder path where captured frames are saved.
	 * @return Pointer to the capture folder string, or nullptr if not set.
	 */
	FString* GetCaptureFolder() const
	{
		return captureFolder;
	}

	/**
	 * Returns the filename of the demo currently being rendered.
	 * @return Pointer to the demo filename string, or nullptr if not set.
	 */
	FString* GetDemoName() const
	{
		return demoFileName;
	}

	/**
	 * Sets the folder where rendered frames will be saved.
	 * @param folder  Absolute path to the output directory.
	 */
	void SetCaptureFolder(const FString& folder);

	/**
	 * Returns the target frame rate for demo rendering.
	 * @return Frames per second for the current capture operation.
	 */
	int GetCaptureFPS() const
	{
		return captureFPS;
	}

	/**
	 * Returns the current demo recording status code.
	 * @return 0 = none, 1 = recording, 2 = ended.
	 */
	int GetDemoState() const
	{
		return demoStatus;
	}

	/** Timestamp recording when the current demo recording started. */
	FDateTime demoStartTime;

	// ------------------------------------------------------------------
	// Demo list for UI
	// ------------------------------------------------------------------

	/**
	 * Returns the list of all known demo filenames. Refreshes from disk if stale.
	 * @return Const reference to the array of demo filename strings.
	 */
	UFUNCTION(BlueprintPure, Category = "Replays")
	const TArray<FString>& GetDemosList();

	/**
	 * Returns a bool array indicating which demos have already been rendered.
	 * Array has same order as GetDemosList().
	 * @return Const reference to the isDemoRendered array.
	 */
	UFUNCTION(BlueprintPure, Category = "Replays")
	const TArray<bool>& GetIsDemoRenderedList()
	{
		return isDemoRendered;
	}

	// ------------------------------------------------------------------
	// Auto rendering
	// ------------------------------------------------------------------

	/** Name of the next replay to render when auto-rendering is enabled. */
	UPROPERTY(BlueprintReadOnly, Category = "fMRI")
	FString replayToRender;

	/**
	 * Whether we should automatically render all demos.
	 * @return true if we should.
	 */
	bool IsAutoRender() const
	{
		return bIsAutoRender;
	}

	/**
	 * Returns the current experiment type loaded from settings.
	 * @return The EExperimentType value for this session.
	 */
	EExperimentType GetExperimentType() const
	{
		return experimentType;
	}

	// ------------------------------------------------------------------
	// OBS integration stub
	// ------------------------------------------------------------------

	/**
	 * Requests OBS to start or stop recording.
	 * Currently a stub; BYO websockets client for OBS control. I used easywsclient
	 * @param startStop  true to start recording, false to stop.
	 * @return Status code from the OBS integration (currently unimplemented).
	 */
	int RequestOBSRecording(bool startStop);

private:

	/** Loaded MRI settings for this session. */
	UPROPERTY()
	UMRISettings* settings;

	/** Experiment type read from settings. */
	EExperimentType experimentType;

	/** True if a replay is currently being played back. */
	bool isReplay = false;
	/** True if a demo is currently being recorded. */
	bool isRecording = false;
	/** Current demo recording status: 0 = none, 1 = recording, 2 = ended. */
	int demoStatus = 0;

	/** If true, the spectator controller should capture frames to disk during playback. */
	bool spectatorToCaptureFrames = false;
	/** Path to the folder where captured frames are saved.*/
	FString* captureFolder = nullptr;
	/** Filename of the demo currently being rendered. */
	FString* demoFileName = nullptr;
	/** 
	 * Frame rate for rendering. The game engine tick will be fixed to match this and be
	 * decoupled from wall time.
	 */
	int captureFPS = 0;

	/** Network replay streamer used for enumerating available replays. */
	TSharedPtr<INetworkReplayStreamer> EnumerateStreamsPtr;
	/** Delegate bound to the replay enumeration completion callback. */
	FOnEnumerateStreamsComplete OnEnumerateStreamsCompleteDelegate;
	/**
	 * Callback invoked when replay enumeration completes.
	 * Converts the raw stream info list into FS_ReplayInfo structs and fires BP_OnFindReplaysComplete.
	 * @param StreamInfos  Array of replay stream metadata returned by the streamer.
	 */
	void OnEnumerateStreamsComplete(const TArray<FNetworkReplayStreamInfo>& StreamInfos);

	/** Delegate bound to the replay deletion completion callback. */
	FOnDeleteFinishedStreamComplete OnDeleteFinishedStreamCompleteDelegate;
	/**
	 * Callback invoked when a replay deletion operation completes.
	 * @param bDeleteSucceeded  true if the replay was deleted successfully.
	 */
	void OnDeleteFinishedStreamComplete(const bool bDeleteSucceeded);

	/** Cached pointer to the player pawn, updated on demand. */
	APawn* playerPawn = nullptr;
	/** Cached name used for name-based player pawn lookup. */
	FName* playerPawnName = nullptr;

	/** Complete list of demo filenames we have on disk. */
	TArray<FString> allDemos;
	/** Array indicating which demos have been rendered. */
	TArray<bool> isDemoRendered;
	/** True if the display resolution has already been applied this session. */
	bool bIsResolutionSet = false;
	/** True if the demos list needs to be refreshed */
	bool bIsDemosListStale = true;
	/** True if we should auto render all demos */
	bool bIsAutoRender = false;

protected:

	/**
	 * Blueprint event fired when replay enumeration completes.
	 * Implement in Blueprint to update a replay-selection UI.
	 * @param AllReplays  Array of FS_ReplayInfo structs describing each available replay.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Replays")
	void BP_OnFindReplaysComplete(const TArray<FS_ReplayInfo>& AllReplays);
};
