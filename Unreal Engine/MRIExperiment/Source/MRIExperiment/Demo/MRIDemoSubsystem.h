// Copyright (c) Gallant Lab. All Rights Reserved.
// Demo/replay management subsystem extracted from UCarlaGameInstance.
// Lives on any UGameInstance so projects do not need to replace their own.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NetworkReplayStreaming.h"

#include "MRIDemoSubsystem.generated.h"

class UMRISettings;

// Replay info struct for blueprint consumption.
USTRUCT(BlueprintType)
struct FS_ReplayInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString ReplayName;

	UPROPERTY(BlueprintReadOnly)
	FString FriendlyName;

	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly)
	int32 LengthInMS;

	UPROPERTY(BlueprintReadOnly)
	bool bIsValid;

	FS_ReplayInfo(FString NewName, FString NewFriendlyName, FDateTime NewTimestamp, int32 NewLengthInMS)
	{
		ReplayName = NewName;
		FriendlyName = NewFriendlyName;
		Timestamp = NewTimestamp;
		LengthInMS = NewLengthInMS;
		bIsValid = true;
	}

	FS_ReplayInfo()
	{
		ReplayName = "Replay";
		FriendlyName = "Replay";
		Timestamp = FDateTime::MinValue();
		LengthInMS = 0;
		bIsValid = false;
	}
};

// Game instance subsystem that manages demo recording, playback, and
// replay enumeration.  Adapted from UCarlaGameInstance so that any
// project can use this functionality without replacing its game instance.
UCLASS()
class MRIEXPERIMENT_API UMRIDemoSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	// ------------------------------------------------------------------
	// Settings access
	// ------------------------------------------------------------------

	UMRISettings& GetSettings();

	const UMRISettings& GetSettings() const;

	UFUNCTION(BlueprintCallable, Category = "MRI")
	UMRISettings* GetMRISettings() { return settings; }

	// ------------------------------------------------------------------
	// Replay state queries
	// ------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Replays")
	bool IsRecordingReplay()
	{
		return isRecording;
	}

	bool IsPlayingReplay()
	{
		return isReplay;
	}

	// ------------------------------------------------------------------
	// Player pawn helpers
	// ------------------------------------------------------------------

	APawn* GetPlayerPawn();

	APawn* GetPlayerPawnByTag();

	APawn* GetPlayerPawnByName();

	// ------------------------------------------------------------------
	// Core replay operations (delegate to UGameInstance)
	// ------------------------------------------------------------------

	void PlayReplay(const FString& replayName);

	void StartRecordingReplay(const FString& name, const FString& friendlyName);

	void StopRecordingReplay();

	// ------------------------------------------------------------------
	// Blueprint wrappers
	// ------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Replays")
	void StartRecordingReplayFromBP(FString ReplayName, FString FriendlyName);

	UFUNCTION(BlueprintCallable, Category = "Replays")
	void StopRecordingReplayFromBP();

	UFUNCTION(BlueprintCallable, Category = "Replays")
	void PlayReplayFromBP(FString ReplayName);

	UFUNCTION(BlueprintCallable, Category = "Replays")
	void FindReplays();

	UFUNCTION(BlueprintCallable, Category = "Replays")
	void RenameReplay(const FString& ReplayName, const FString& NewFriendlyReplayName);

	UFUNCTION(BlueprintCallable, Category = "Replays")
	void DeleteReplay(const FString& ReplayName);

	// ------------------------------------------------------------------
	// Demo rendering
	// ------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Replays")
	void DemoRenderFrames(const FString& demoName, int fps);

	bool CaptureFrames() const
	{
		return spectatorToCaptureFrames;
	}

	FString* GetCaptureFolder() const
	{
		return captureFolder;
	}

	FString* GetDemoName() const
	{
		return demoFileName;
	}

	void SetCaptureFolder(const FString& folder);

	int GetCaptureFPS() const
	{
		return captureFPS;
	}

	int GetDemoState() const
	{
		return demoStatus;
	}

	FDateTime demoStartTime;

	// ------------------------------------------------------------------
	// Demo list for UI
	// ------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Replays")
	const TArray<FString>& GetDemosList();

	UFUNCTION(BlueprintPure, Category = "Replays")
	const TArray<bool>& GetIsDemoRenderedList()
	{
		return isDemoRendered;
	}

	// ------------------------------------------------------------------
	// Auto rendering
	// ------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, Category = "fMRI")
	FString replayToRender;

	bool IsAutoRender() const
	{
		return bIsAutoRender;
	}

	EExperimentType GetExperimentType() const
	{
		return experimentType;
	}

	// ------------------------------------------------------------------
	// OBS integration stub
	// ------------------------------------------------------------------

	int RequestOBSRecording(bool startStop);

private:

	UPROPERTY()
	UMRISettings* settings;

	EExperimentType experimentType;

	bool isReplay = false;
	bool isRecording = false;
	int demoStatus = 0;	// 0 = none, 1 = recording, 2 = ended

	bool spectatorToCaptureFrames = false;
	FString* captureFolder = nullptr;
	FString* demoFileName = nullptr;
	int captureFPS = 0;

	// Replay enumeration streamer
	TSharedPtr<INetworkReplayStreamer> EnumerateStreamsPtr;
	FOnEnumerateStreamsComplete OnEnumerateStreamsCompleteDelegate;
	void OnEnumerateStreamsComplete(const TArray<FNetworkReplayStreamInfo>& StreamInfos);

	// Replay deletion
	FOnDeleteFinishedStreamComplete OnDeleteFinishedStreamCompleteDelegate;
	void OnDeleteFinishedStreamComplete(const bool bDeleteSucceeded);

	APawn* playerPawn = nullptr;
	FName* playerPawnName = nullptr;

	// Demo list state
	TArray<FString> allDemos;
	TArray<bool> isDemoRendered;
	bool bIsResolutionSet = false;
	bool bIsDemosListStale = true;
	bool bIsAutoRender = false;

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Replays")
	void BP_OnFindReplaysComplete(const TArray<FS_ReplayInfo>& AllReplays);
};
