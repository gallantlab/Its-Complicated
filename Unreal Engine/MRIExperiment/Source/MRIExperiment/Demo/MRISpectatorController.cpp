// Copyright (c) Gallant Lab. All Rights Reserved.

#include "MRIExperiment.h"
#include "Demo/MRISpectatorController.h"
#include "Demo/MRIDemoSubsystem.h"
#include "State/MRISubjectState.h"
#include "Agent/MRIAgentComponent.h"
#include "Settings/MRISettings.h"

#include "Engine/DemoNetDriver.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/WorldSettings.h"

#include <iostream>
#include <fstream>

using std::ofstream;
using std::endl;


AMRISpectatorController::AMRISpectatorController(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	bShouldPerformFullTickWhenPaused = true;

	sensors = new TArray<AMRIFrameCaptureCamera*>();
	captureFrames = false;
	frameNumber = 0;
	playerAgentID = -1;

	UE_LOG(LogMRI, Log, TEXT("MRI spectator controller spawned"));
}


AMRISpectatorController::~AMRISpectatorController()
{
}


void AMRISpectatorController::PostActorCreated()
{
	Super::PostActorCreated();
	UE_LOG(LogMRI, Log, TEXT("MRI spectator controller PostActorCreated called"));

	UGameInstance* gameInstance = GetWorld()->GetGameInstance();
	demoSubsystem = gameInstance->GetSubsystem<UMRIDemoSubsystem>();

	if (demoSubsystem && demoSubsystem->IsPlayingReplay())
	{
		TrySetViewTargetToPlayer();

		if (demoSubsystem->CaptureFrames())
		{
			SetFrameCaptureFolder(*(demoSubsystem->GetCaptureFolder()));
			fps = demoSubsystem->GetCaptureFPS();
			currentTime = -1.0;
			timeStep = 1.0 / fps;
			delay = 5.0;
			worldSettings = GetWorldSettings();

			FixTimeStep(true);

			if (!worldSettings)
			{
				UE_LOG(LogMRI, Log, TEXT("World settings is null pointer"));
				return;
			}
			if (!PlayerState)
			{
				UE_LOG(LogMRI, Log, TEXT("No player state. Using the reference pawn's."));
				if (referencePawn && referencePawn->PlayerState)
					PlayerState = referencePawn->PlayerState;
				else
					UE_LOG(LogMRI, Log, TEXT("Reference pawn does not have a player state either. TTLs will not be read out."));
			}
			if (!worldSettings->Pauser)
				worldSettings->Pauser = PlayerState;
			demoNetDriver = GetWorld()->DemoNetDriver;

			captureFrames = true;
		}
	}
}


void AMRISpectatorController::BeginPlayingState()
{
	Super::BeginPlayingState();
	UE_LOG(LogMRI, Log, TEXT("MRI spectator controller BeginPlayingState"));
	TrySetViewTargetToPlayer();
}


APawn* AMRISpectatorController::FindTaggedPlayerPawn()
{
	UE_LOG(LogMRI, Log, TEXT("Spectator controller trying to find a tagged player pawn"));
	APawn* pawn = nullptr;
	for (TActorIterator<APawn> pawnIterator(GetWorld()); pawnIterator; ++pawnIterator)
	{
		if (pawnIterator->ActorHasTag(MRIPlayerTag))
		{
			UE_LOG(LogMRI, Log, TEXT("Player tag found, actor is %s"), *pawnIterator->GetName());
			pawn = *pawnIterator;
			break;
		}
	}
	return pawn;
}


void AMRISpectatorController::TrySetViewTargetToPlayer()
{
	UE_LOG(LogMRI, Log, TEXT("Trying to set view target to player"));
	if (GetWorld() == nullptr)
	{
		needReference = true;
		return;
	}

	UGameInstance* gameInstance = GetGameInstance();
	if (!gameInstance)
	{
		UE_LOG(LogMRI, Log, TEXT("Has world but not game instance"));
		return;
	}

	demoSubsystem = gameInstance->GetSubsystem<UMRIDemoSubsystem>();
	if (!demoSubsystem)
	{
		UE_LOG(LogMRI, Error, TEXT("Could not find demo subsystem"));
		return;
	}

	APawn* playerPawn = demoSubsystem->GetPlayerPawn();
	if (playerPawn == nullptr)
	{
		UE_LOG(LogMRI, Log, TEXT("No player pawn found"));
		return;
	}

	UE_LOG(LogMRI, Log, TEXT("Player pawn is %s"), *playerPawn->GetName());
	this->AttachToActor(playerPawn, FAttachmentTransformRules::SnapToTargetIncludingScale);
	this->referencePawn = playerPawn;
	this->referencePlayerState = Cast<AMRISubjectState>(referencePawn->PlayerState);
	if (!referencePlayerState)
		UE_LOG(LogMRI, Log, TEXT("Reference pawn player state is a null pointer"));
	UE_LOG(LogMRI, Log, TEXT("View target set to %s"), *playerPawn->GetName());
	needReference = false;

	if (demoSubsystem->IsPlayingReplay())
	{
		// Sensor creation is project-specific. This base class initializes an empty
		// sensor list. Projects should override or extend to add their own sensors.
		if (sensors->Num() > 0)
			sensors->Empty();
		UE_LOG(LogMRI, Log, TEXT("Sensor list initialized. Projects should add experiment-specific sensors."));

		frameStates = new TArray<MRIExperimentState*>();
	}

	TArray<AActor*>* attachedActors = new TArray<AActor*>();
	playerPawn->GetAttachedActors(*attachedActors);
	for (int i = 0; i < attachedActors->Num(); i++)
		UE_LOG(LogMRI, Log, TEXT("Found attached actor %s"), *(*attachedActors)[i]->GetName());

	CaptureSensors();
}


void AMRISpectatorController::CaptureSensors()
{
	if (sensors == nullptr)
	{
		UE_LOG(LogMRI, Error, TEXT("Null sensors list"));
		return;
	}
	for (int i = 0; i < sensors->Num(); i++)
		(*sensors)[i]->SaveNextFrame();
}


void AMRISpectatorController::AddExperimentState(MRIExperimentState* state)
{
	frameStates->Add(state);
}


void AMRISpectatorController::Tick(float deltaTime)
{
	if (captureFrames)
	{
		if (delay > 0)
		{
			if (!worldSettings->Pauser)
				worldSettings->Pauser = PlayerState;
			delay -= deltaTime;

			if (referencePawn && referencePawn->PlayerState)
			{
				UE_LOG(LogMRI, Log, TEXT("Player state found during initial ticks"));
				referencePlayerState = Cast<AMRISubjectState>(referencePawn->PlayerState);
			}
		}
		else
		{
			if (worldSettings->Pauser)
			{
				worldSettings->Pauser = nullptr;
				UE_LOG(LogMRI, Log, TEXT("Begin rendering frames"));
				frameNumber = 0;
			}

			if (referencePlayerState)
			{
				// if you need to do something here with the player state
			}
			else
			{
				UE_LOG(LogMRI, Log, TEXT("Time %f, dt %f, no player state available"), demoNetDriver->DemoCurrentTime, deltaTime);
			}

			CaptureSensors();
			if (referencePlayerState)
				LogExperimentState();

			if (currentTime == demoNetDriver->DemoCurrentTime)
			{
				captureFrames = false;
				FixTimeStep(false);
				if (referencePlayerState)
					SaveExperimentLog();
				UE_LOG(LogMRI, Log, TEXT("End replay and end rendering frames"));
				demoSubsystem->isReplay = false;
				stopDemo = true;
			}
			else
			{
				currentTime = demoNetDriver->DemoCurrentTime;
			}

			frameNumber++;
		}
	}
	else if (stopDemo)
	{
		demoNetDriver->StopDemo();
		GetWorld()->DestroyDemoNetDriver();
		RestartLevel();
	}

	Super::Tick(deltaTime);
}


void AMRISpectatorController::SaveExperimentLog()
{
#define FORMAT_TO_CHAR_ARRAY(string, ...) TCHAR_TO_ANSI(*FString::Printf(TEXT(string), ##__VA_ARGS__))

	IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString fileName = platformFile.ConvertToAbsolutePathForExternalAppForWrite(
		*FPaths::Combine(FPaths::ProjectSavedDir(),
			FString::Printf(TEXT("%s-log.xml"), **(demoSubsystem->GetDemoName()))));

	ofstream logFile;
	logFile.open(TCHAR_TO_ANSI(*fileName));

	int indentation = 0;

	logFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
	logFile << "<Experiment>" << endl;
	indentation++;

	MRI_WRITE_INDENTS(logFile, indentation)
	logFile << MRI_WRITE_SINGLE_TAG("Experiment-Type",
		TCHAR_TO_ANSI(*UExperimentType::ToString(experimentType))) << endl;

	MRI_WRITE_INDENTS(logFile, indentation)
	logFile << MRI_WRITE_SINGLE_TAG("Player-ID", playerAgentID) << endl;

	MRI_WRITE_INDENTS(logFile, indentation)
	logFile << MRI_WRITE_SINGLE_TAG("Capture-FPS", demoSubsystem->GetCaptureFPS()) << endl;

	MRI_WRITE_INDENTS(logFile, indentation)
	logFile << MRI_WRITE_SINGLE_TAG("Seconds-To-Run-Start", referencePlayerState->GetSecondsToStartOfRun()) << endl;

	// Per-frame states
	if (frameStates)
	{
		UE_LOG(LogMRI, Log, TEXT("%d frames to save"), frameStates->Num());
		int percentStep = frameStates->Num() / 100;
		percentStep = percentStep ? percentStep : 1;

		for (int frame = 0; frame < frameStates->Num(); frame++)
		{
			(*frameStates)[frame]->WriteToFile(logFile, indentation);

			if (frame % percentStep == 0)
				UE_LOG(LogMRI, Log, TEXT("%d/%d frames written"), frame + 1, frameStates->Num() + 1);
		}
	}

	indentation--;
	logFile << "</Experiment>" << endl;
	logFile.close();
	UE_LOG(LogMRI, Log, TEXT("File saved to %s"), *fileName);

	if (demoSubsystem->IsAutoRender())
		FGenericPlatformMisc::RequestExit(false);
}


void AMRISpectatorController::SetFrameCaptureFolder(const FString& folder)
{
	if (sensors == nullptr)
	{
		UE_LOG(LogMRI, Error, TEXT("Tried to set frame capture folder but the sensor list is null"));
		return;
	}
	for (int i = 0; i < sensors->Num(); i++)
		(*sensors)[i]->SetSaveFolder(folder);
}


void AMRISpectatorController::FixTimeStep(bool fix)
{
	FApp::SetFixedDeltaTime(fix ? timeStep : 0);
	FApp::SetBenchmarking(fix);
	FApp::SetUseFixedTimeStep(fix);
}


void AMRISpectatorController::SpawnLoggerComponent()
{
	UGameInstance* gameInstance = GetWorld()->GetGameInstance();
	demoSubsystem = gameInstance->GetSubsystem<UMRIDemoSubsystem>();

	if (demoSubsystem)
		experimentType = demoSubsystem->GetExperimentType();
	else
		experimentType = EExperimentType::Other;

	UE_LOG(LogMRI, Log, TEXT("Base MRI logger component spawned"));
	loggerComponent = NewObject<UMRILoggerComponent>(this);
	loggerComponent->SetParentController(this);
}


void AMRISpectatorController::LogExperimentState()
{
	if (!loggerComponent)
		SpawnLoggerComponent();
	loggerComponent->LogFrame();
}


void AMRISpectatorController::SetReferencePawn(APawn* referencePawn)
{
	this->referencePawn = referencePawn;
}
