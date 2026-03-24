// Copyright (c) Gallant Lab. All Rights Reserved.
// Demo/replay management subsystem extracted from UCarlaGameInstance.

#include "MRIExperiment.h"
#include "Demo/MRIDemoSubsystem.h"
#include "Settings/MRISettings.h"

#include "Runtime/NetworkReplayStreaming/NullNetworkReplayStreaming/Public/NullNetworkReplayStreaming.h"
#include "Runtime/Core/Public/HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "NetworkVersion.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void UMRIDemoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	settings = NewObject<UMRISettings>(this, TEXT("MRISettings"));
	check(settings != nullptr);
	settings->LoadSettings();
	settings->LogSettings();

	playerPawn = nullptr;
	playerPawnName = nullptr;
	demoStartTime = FDateTime::Now();
	allDemos = TArray<FString>();
	bIsAutoRender = settings->RenderAll > 0;
	experimentType = settings->GetExperimentType();

	// Create a replay streamer for FindReplays() and DeleteReplay()
	EnumerateStreamsPtr = FNetworkReplayStreaming::Get().GetFactory().CreateReplayStreamer();

	// Link FindReplays() delegate
	OnEnumerateStreamsCompleteDelegate = FOnEnumerateStreamsComplete::CreateUObject(this, &UMRIDemoSubsystem::OnEnumerateStreamsComplete);

	// Link DeleteReplay() delegate
	OnDeleteFinishedStreamCompleteDelegate = FOnDeleteFinishedStreamComplete::CreateUObject(this, &UMRIDemoSubsystem::OnDeleteFinishedStreamComplete);

	UE_LOG(LogMRI, Log, TEXT("MRIDemoSubsystem initialized"));
}

void UMRIDemoSubsystem::Deinitialize()
{
	if (!isReplay)
	{
		SaveSubjectStateFile();
	}

	Super::Deinitialize();
}

// ---------------------------------------------------------------------------
// Settings access
// ---------------------------------------------------------------------------

UMRISettings& UMRIDemoSubsystem::GetSettings()
{
	check(settings != nullptr);
	return *settings;
}

const UMRISettings& UMRIDemoSubsystem::GetSettings() const
{
	check(settings != nullptr);
	return *settings;
}

// ---------------------------------------------------------------------------
// Player pawn helpers
// ---------------------------------------------------------------------------

APawn* UMRIDemoSubsystem::GetPlayerPawn()
{
	// Try by cached name first, fall back to tag search
	APawn* result = GetPlayerPawnByName();
	result = result ? result : GetPlayerPawnByTag();
	return result;
}

APawn* UMRIDemoSubsystem::GetPlayerPawnByTag()
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
		if (actorIterator->ActorHasTag(MRIPlayerTag))
		{
			result = *actorIterator;
			playerPawnName = new FName(actorIterator->GetFName());
			numberOfPlayerTags++;
		}
	}
	return result;
}

APawn* UMRIDemoSubsystem::GetPlayerPawnByName()
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

void UMRIDemoSubsystem::PlayReplay(const FString& replayName)
{
	UE_LOG(LogMRI, Log, TEXT("Starting demofile replay"));
	isReplay = true;

	GetGameInstance()->PlayReplay(replayName);
	UE_LOG(LogMRI, Log, TEXT("UGameInstance PlayReplay call has returned"));

	playerPawn = nullptr;
	demoFileName = new FString(replayName);

	if (GetPlayerPawn() != nullptr)
	{
		UE_LOG(LogMRI, Log, TEXT("Found a pawn tagged as player"));
	}

	bIsDemosListStale = true;
}

void UMRIDemoSubsystem::StartRecordingReplay(const FString& name, const FString& friendlyName)
{
	if (!name.IsEmpty())
	{
		GetGameInstance()->StartRecordingReplay(name, friendlyName);
	}
	else
	{
		FString timestampName = FDateTime::Now().ToString().Replace(TEXT("."), TEXT("-"));
		UE_LOG(LogMRI, Log, TEXT("No demo name given, using %s"), *timestampName);
		GetGameInstance()->StartRecordingReplay(timestampName, friendlyName);
	}

	UE_LOG(LogMRI, Log, TEXT("Demo %s recording started %s"), *name, *FDateTime::Now().ToString());
	isRecording = true;
	isReplay = false;
	demoStatus = 1;
	demoStartTime = FDateTime::Now();
	bIsDemosListStale = true;

	int OBSRequestResult = RequestOBSRecording(true);
	UE_LOG(LogMRI, Log, TEXT("OBS recording request result: %d"), OBSRequestResult);
}

void UMRIDemoSubsystem::StopRecordingReplay()
{
	GetGameInstance()->StopRecordingReplay();
	isRecording = false;
	demoStatus = 2;
	bIsDemosListStale = true;

	int OBSRequestResult = RequestOBSRecording(false);
	UE_LOG(LogMRI, Log, TEXT("OBS stop-recording request result: %d"), OBSRequestResult);
	UE_LOG(LogMRI, Log, TEXT("Demo stop called"));
}

// ---------------------------------------------------------------------------
// Blueprint wrappers
// ---------------------------------------------------------------------------

void UMRIDemoSubsystem::StartRecordingReplayFromBP(FString ReplayName, FString FriendlyName)
{
	StartRecordingReplay(ReplayName, FriendlyName);
}

void UMRIDemoSubsystem::StopRecordingReplayFromBP()
{
	StopRecordingReplay();
}

void UMRIDemoSubsystem::PlayReplayFromBP(FString ReplayName)
{
	PlayReplay(ReplayName);
}

// ---------------------------------------------------------------------------
// Demo rendering
// ---------------------------------------------------------------------------

void UMRIDemoSubsystem::DemoRenderFrames(const FString& demoName, int fps)
{
	// Chop off the " (R)" suffix on an already rendered demo
	const FString actualDemoName = demoName.EndsWith(FString(" (R)")) ? demoName.LeftChop(4) : demoName;
	PlayReplay(actualDemoName);

	// Spectator controller does not exist here yet, so we leave a flag
	// for the controller to eventually find.
	spectatorToCaptureFrames = true;
	captureFolder = new FString(actualDemoName);
	captureFPS = fps;

	// Immediately pause the replay
	APawn* pawn = GetPlayerPawn();
	if (pawn)
	{
		AWorldSettings* worldSettings = pawn->GetWorldSettings();
		if (worldSettings && pawn->PlayerState)
		{
			worldSettings->Pauser = pawn->PlayerState;
		}
	}
}

void UMRIDemoSubsystem::SetCaptureFolder(const FString& folder)
{
	captureFolder = new FString(folder);
	UE_LOG(LogMRI, Log, TEXT("Capture folder set to %s"), *folder);
}

// ---------------------------------------------------------------------------
// Replay enumeration
// ---------------------------------------------------------------------------

void UMRIDemoSubsystem::FindReplays()
{
	if (bIsDemosListStale && EnumerateStreamsPtr.Get())
	{
		EnumerateStreamsPtr.Get()->EnumerateStreams(FNetworkReplayVersion(), FString(), FString(), OnEnumerateStreamsCompleteDelegate);
	}
}

void UMRIDemoSubsystem::OnEnumerateStreamsComplete(const TArray<FNetworkReplayStreamInfo>& StreamInfos)
{
	IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();

	bool hasUnrenderedReplays = false;
	replayToRender = FString();

	allDemos.Empty();
	isDemoRendered.Empty();

	FString savedFileName;

	for (FNetworkReplayStreamInfo streamInfo : StreamInfos)
	{
		if (!streamInfo.bIsLive)
		{
			UE_LOG(LogMRI, Log, TEXT("Replay %s"), *(streamInfo.Name));
			savedFileName = platformFile.ConvertToAbsolutePathForExternalAppForWrite(
				*FPaths::Combine(FPaths::ProjectSavedDir(), FString::Printf(TEXT("%s-positions.xml"), *(streamInfo.Name))));

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

	UE_LOG(LogMRI, Log, TEXT("%d replays found"), allDemos.Num());
	bIsDemosListStale = false;
}

// ---------------------------------------------------------------------------
// Replay rename and delete
// ---------------------------------------------------------------------------

void UMRIDemoSubsystem::RenameReplay(const FString& ReplayName, const FString& NewFriendlyReplayName)
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

void UMRIDemoSubsystem::DeleteReplay(const FString& ReplayName)
{
	if (EnumerateStreamsPtr.Get())
	{
		EnumerateStreamsPtr.Get()->DeleteFinishedStream(ReplayName, OnDeleteFinishedStreamCompleteDelegate);
	}
}

void UMRIDemoSubsystem::OnDeleteFinishedStreamComplete(const bool bDeleteSucceeded)
{
	FindReplays();
}

// ---------------------------------------------------------------------------
// OBS integration stub
// ---------------------------------------------------------------------------

int UMRIDemoSubsystem::RequestOBSRecording(bool startStop)
{
	// I used easywsclient for OBS websocket
	// communication nad hard-coded addresses and requests here
	// you can either just this or do it yourself for more flexibility

	/*
	using easywsclient::WebSocket;
	WebSocket::pointer client = WebSocket::from_url("ws://localhost:4444");
	if (client == NULL)
	{
		UE_LOG(LogFMRI, Error, TEXT("No websocket connection to OBS"));
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

	UE_LOG(LogMRI, Warning, TEXT("RequestOBSRecording: OBS websocket integration is not available. ")
		TEXT("Projects must provide their own websocket client for OBS control."));
	return -1;
}

// ---------------------------------------------------------------------------
// Demo list for UI
// ---------------------------------------------------------------------------

const TArray<FString>& UMRIDemoSubsystem::GetDemosList()
{
	if (bIsDemosListStale)
	{
		FindReplays();
	}
	return allDemos;
}