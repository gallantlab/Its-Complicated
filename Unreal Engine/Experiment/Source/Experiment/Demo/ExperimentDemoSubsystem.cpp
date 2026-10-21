// Copyright (c) Gallant Lab. All Rights Reserved.
// Demo/replay management subsystem extracted from UCarlaGameInstance.

#include "ExperimentDemoSubsystem.h"

#include "NullNetworkReplayStreaming.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/NetworkVersion.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

#include "EngineUtils.h"

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void UExperimentDemoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	playerPawn = nullptr;
	playerPawnName = nullptr;
	demoStartTime = FDateTime::Now();
	allDemos = TArray<FString>();

	// Create a replay streamer for FindReplays() and DeleteReplay()
	EnumerateStreamsPtr = FNetworkReplayStreaming::Get().GetFactory().CreateReplayStreamer();

	// Link FindReplays() delegate
	OnEnumerateStreamsCompleteDelegate = FEnumerateStreamsCallback::CreateUObject(this, &UExperimentDemoSubsystem::OnEnumerateStreamsComplete);

	// Link DeleteReplay() delegate
	OnDeleteFinishedStreamCompleteDelegate = FDeleteFinishedStreamCallback::CreateUObject(this, &UExperimentDemoSubsystem::OnDeleteFinishedStreamComplete);

	UE_LOG(LogExperiment, Log, TEXT("ExperimentDemoSubsystem initialized"));
}

void UExperimentDemoSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ---------------------------------------------------------------------------
// Settings access
// ---------------------------------------------------------------------------

void UExperimentDemoSubsystem::ReadSettings(UExperimentSettings* settings)
{	
	bIsAutoRender = settings->RenderAll > 0;
	experimentType = settings->GetExperimentType();
}

// ---------------------------------------------------------------------------
// Player pawn helpers
// ---------------------------------------------------------------------------

APawn* UExperimentDemoSubsystem::GetPlayerPawn()
{
	// Try by cached name first, fall back to tag search
	APawn* result = GetPlayerPawnByName();
	result = result ? result : GetPlayerPawnByTag();
	return result;
}

APawn* UExperimentDemoSubsystem::GetPlayerPawnByTag()
{
	UWorld* world = GetGameInstance()->GetWorld();
	if (!world)
	{
		return nullptr;
	}

	APawn* result = nullptr;
	int numberOfPlayerTags = 0;
	for (TActorIterator<APawn> actorIterator(world); actorIterator; ++actorIterator)
	{
		if (actorIterator->ActorHasTag(ExperimentPlayerTag))
		{
			result = *actorIterator;
			playerPawnName = new FName(actorIterator->GetFName());
			numberOfPlayerTags++;
		}
	}
	return result;
}

APawn* UExperimentDemoSubsystem::GetPlayerPawnByName()
{
	if (!playerPawnName)
	{
		return nullptr;
	}

	UWorld* world = GetGameInstance()->GetWorld();
	if (!world)
	{
		return nullptr;
	}

	for (TActorIterator<APawn> actorIterator(world); actorIterator; ++actorIterator)
	{
		FName thisName = actorIterator->GetFName();
		if (thisName == *playerPawnName)
		{
			return *actorIterator;
		}
	}
	return nullptr;
}

// ---------------------------------------------------------------------------
// Core replay operations
// ---------------------------------------------------------------------------

void UExperimentDemoSubsystem::PlayReplay(const FString& replayName)
{
	UE_LOG(LogExperiment, Log, TEXT("Starting demofile replay"));
	isReplay = true;

	GetGameInstance()->PlayReplay(replayName);
	UE_LOG(LogExperiment, Log, TEXT("UGameInstance PlayReplay call has returned"));

	playerPawn = nullptr;
	demoFileName = new FString(replayName);

	if (GetPlayerPawn() != nullptr)
	{
		UE_LOG(LogExperiment, Log, TEXT("Found a pawn tagged as player"));
	}

	bIsDemosListStale = true;
}

void UExperimentDemoSubsystem::StartRecordingReplay(const FString& name, const FString& friendlyName)
{
	if (!name.IsEmpty())
	{
		GetGameInstance()->StartRecordingReplay(name, friendlyName);
	}
	else
	{
		FString timestampName = FDateTime::Now().ToString().Replace(TEXT("."), TEXT("-"));
		UE_LOG(LogExperiment, Log, TEXT("No demo name given, using %s"), *timestampName);
		GetGameInstance()->StartRecordingReplay(timestampName, friendlyName);
	}

	UE_LOG(LogExperiment, Log, TEXT("Demo %s recording started %s"), *name, *FDateTime::Now().ToString());
	isRecording = true;
	isReplay = false;
	demoStatus = 1;
	demoStartTime = FDateTime::Now();
	bIsDemosListStale = true;

	int OBSRequestResult = RequestOBSRecording(true);
	UE_LOG(LogExperiment, Log, TEXT("OBS recording request result: %d"), OBSRequestResult);
}

void UExperimentDemoSubsystem::StopRecordingReplay()
{
	GetGameInstance()->StopRecordingReplay();
	isRecording = false;
	demoStatus = 2;
	bIsDemosListStale = true;

	int OBSRequestResult = RequestOBSRecording(false);
	UE_LOG(LogExperiment, Log, TEXT("OBS stop-recording request result: %d"), OBSRequestResult);
	UE_LOG(LogExperiment, Log, TEXT("Demo stop called"));
}

// ---------------------------------------------------------------------------
// Blueprint wrappers
// ---------------------------------------------------------------------------

void UExperimentDemoSubsystem::StartRecordingReplayFromBP(FString ReplayName, FString FriendlyName)
{
	StartRecordingReplay(ReplayName, FriendlyName);
}

void UExperimentDemoSubsystem::StopRecordingReplayFromBP()
{
	StopRecordingReplay();
}

void UExperimentDemoSubsystem::PlayReplayFromBP(FString ReplayName)
{
	PlayReplay(ReplayName);
}

// ---------------------------------------------------------------------------
// Demo rendering
// ---------------------------------------------------------------------------

void UExperimentDemoSubsystem::DemoRenderFrames(const FString& demoName, int fps)
{
	// Chop off the " (R)" suffix on an already rendered demo
	const FString actualDemoName = demoName.EndsWith(FString(" (R)")) ? demoName.LeftChop(4) : demoName;
	PlayReplay(actualDemoName);

	// Spectator controller does not exist here yet, so we leave a flag
	// for the controller to eventually find.
	spectatorToCaptureFrames = true;
	// ProjectSavedDir resolves to the project's Saved folder, or to the user's
	// writable app-data folder for installed builds.
	captureFolder = new FString(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Renders"), actualDemoName));
	captureFPS = fps;

	// Immediately pause the replay
	APawn* pawn = GetPlayerPawn();
	if (pawn)
	{
		AWorldSettings* worldSettings = pawn->GetWorldSettings();
		if (worldSettings && pawn->GetPlayerState())
		{
			worldSettings->SetPauserPlayerState(pawn->GetPlayerState());
		}
	}
}

void UExperimentDemoSubsystem::SetCaptureFolder(const FString& folder)
{
	captureFolder = new FString(folder);
	UE_LOG(LogExperiment, Log, TEXT("Capture folder set to %s"), *folder);
}

// ---------------------------------------------------------------------------
// Replay enumeration
// ---------------------------------------------------------------------------

void UExperimentDemoSubsystem::FindReplays()
{
	if (bIsDemosListStale && EnumerateStreamsPtr.Get())
	{
		EnumerateStreamsPtr.Get()->EnumerateStreams(FNetworkReplayVersion(), INDEX_NONE, FString(), TArray<FString>(), OnEnumerateStreamsCompleteDelegate);
	}
}

void UExperimentDemoSubsystem::OnEnumerateStreamsComplete(const FEnumerateStreamsResult& Result)
{
	IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();

	bool hasUnrenderedReplays = false;
	replayToRender = FString();

	allDemos.Empty();
	isDemoRendered.Empty();

	FString savedFileName;
	TArray<FReplayInfo> allReplays;

	for (FNetworkReplayStreamInfo streamInfo : Result.FoundStreams)
	{
		if (!streamInfo.bIsLive)
		{
			UE_LOG(LogExperiment, Log, TEXT("Replay %s"), *(streamInfo.Name));
			allReplays.Add(FReplayInfo(streamInfo.Name, streamInfo.FriendlyName, streamInfo.Timestamp, streamInfo.LengthInMS));
			savedFileName = platformFile.ConvertToAbsolutePathForExternalAppForWrite(
				*FPaths::Combine(FPaths::ProjectSavedDir(), FString::Printf(TEXT("%s-log.xml"), *(streamInfo.Name))));

			if (FPaths::FileExists(savedFileName))
			{
				allDemos.AddUnique(streamInfo.Name.Append(FString(" (R)")));
				isDemoRendered.Add(true);
			}
			else
			{
				allDemos.AddUnique(streamInfo.Name);
				isDemoRendered.Add(false);
				hasUnrenderedReplays = true;
				replayToRender = streamInfo.Name;
			}
		}
	}

	allDemos.Sort([](const FString& lhs, const FString& rhs) { return lhs.Compare(rhs) < 0; });

	UE_LOG(LogExperiment, Log, TEXT("%d replays found"), allDemos.Num());
	bIsDemosListStale = false;

	BP_OnFindReplaysComplete(allReplays);
}

// ---------------------------------------------------------------------------
// Replay rename and delete
// ---------------------------------------------------------------------------

void UExperimentDemoSubsystem::RenameReplay(const FString& ReplayName, const FString& NewFriendlyReplayName)
{
	FNullReplayInfo Info;

	const FString DemoPath = FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("Demos/"));
	const FString StreamDirectory = FPaths::Combine(*DemoPath, *ReplayName);
	const FString StreamFullBaseFilename = FPaths::Combine(*StreamDirectory, *ReplayName);
	const FString InfoFilename = StreamFullBaseFilename + TEXT(".replayinfo");

	TUniquePtr<FArchive> InfoFileArchive(IFileManager::Get().CreateFileReader(*InfoFilename));

	if (InfoFileArchive.IsValid() && InfoFileArchive->TotalSize() != 0)
	{
		FString JsonString;
		*InfoFileArchive << JsonString;

		Info.FromJson(JsonString);
		Info.bIsValid = true;

		InfoFileArchive->Close();
	}

	Info.FriendlyName = NewFriendlyReplayName;

	TUniquePtr<FArchive> ReplayInfoFileArchive(IFileManager::Get().CreateFileWriter(*InfoFilename));

	if (ReplayInfoFileArchive.IsValid())
	{
		FString JsonString = Info.ToJson();
		*ReplayInfoFileArchive << JsonString;

		ReplayInfoFileArchive->Close();
	}
}

void UExperimentDemoSubsystem::DeleteReplay(const FString& ReplayName)
{
	if (EnumerateStreamsPtr.Get())
	{
		EnumerateStreamsPtr.Get()->DeleteFinishedStream(ReplayName, OnDeleteFinishedStreamCompleteDelegate);
	}
}

void UExperimentDemoSubsystem::OnDeleteFinishedStreamComplete(const FDeleteFinishedStreamResult& Result)
{
	FindReplays();
}

// ---------------------------------------------------------------------------
// OBS integration stub
// ---------------------------------------------------------------------------

int UExperimentDemoSubsystem::RequestOBSRecording(bool startStop)
{
	// I used easywsclient for OBS websocket
	// communication nad hard-coded addresses and requests here
	// you can either just this or do it yourself for more flexibility

	/*
	using easywsclient::WebSocket;
	WebSocket::pointer client = WebSocket::from_url("ws://localhost:4444");
	if (client == NULL)
	{
		UE_LOG(LogFExperiment, Error, TEXT("No websocket connection to OBS"));
		return -1;
	}
	if (startStop)
		client->send("{\"request-type\": \"StartRecording\", \"message-id\": 0}");
	else
		client->send("{\"request-type\": \"StopRecording\", \"message-id\": 0}");
	client->poll();
	client->close();
	delete client;
	return 0;
	*/

	UE_LOG(LogExperiment, Warning, TEXT("RequestOBSRecording: OBS websocket integration is not available. ")
		TEXT("Projects must provide their own websocket client for OBS control."));
	return -1;
}

// ---------------------------------------------------------------------------
// Demo list for UI
// ---------------------------------------------------------------------------

const TArray<FString>& UExperimentDemoSubsystem::GetDemosList()
{
	allDemos.Empty();
	isDemoRendered.Empty();

	TArray<FString> replayFiles;
	IFileManager::Get().FindFiles(replayFiles,
		*FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Demos"), TEXT("*.replay")), true, false);
	replayFiles.Sort();

	for (const FString& replayFile : replayFiles)
	{
		const FString name = FPaths::GetBaseFilename(replayFile);
		const FString logFileName = FPaths::Combine(FPaths::ProjectSavedDir(),
			FString::Printf(TEXT("%s-log.xml"), *name));

		if (FPaths::FileExists(logFileName))
		{
			allDemos.Add(name + TEXT(" (R)"));
			isDemoRendered.Add(true);
		}
		else
		{
			allDemos.Add(name);
			isDemoRendered.Add(false);
		}
	}
	return allDemos;
}