// Copyright (c) Gallant Lab. All Rights Reserved.

#include "ExperimentSpectatorController.h"

#include "Demo/ExperimentDemoSubsystem.h"
#include "State/ExperimentSubjectState.h"
#include "Agent/ExperimentAgentComponent.h"
#include "Camera/SemSegTagger.h"
#include "Settings/ExperimentSettings.h"
#include "Settings/ExperimentSettingsSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/DemoNetDriver.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/PlatformFileManager.h"

#include <iostream>
#include <fstream>

using std::ofstream;
using std::endl;


AExperimentSpectatorController::AExperimentSpectatorController(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	bShouldPerformFullTickWhenPaused = true;

	// Keep the engine from resetting the view target to the spectator pawn
	// when it is possessed; the view target is the replayed player pawn.
	bAutoManageActiveCameraTarget = false;

	captureFrames = false;
	frameNumber = 0;
	playerAgentID = -1;

	UE_LOG(LogExperiment, Log, TEXT("Experiment spectator controller spawned"));
}


AExperimentSpectatorController::~AExperimentSpectatorController()
{
	for (int i = 0; i < frameStates.Num(); i++)
		delete frameStates[i];
}


void AExperimentSpectatorController::PostActorCreated()
{
	Super::PostActorCreated();
	UE_LOG(LogExperiment, Log, TEXT("Experiment spectator controller PostActorCreated called"));

	UGameInstance* gameInstance = GetWorld()->GetGameInstance();
	demoSubsystem = gameInstance->GetSubsystem<UExperimentDemoSubsystem>();

	if (demoSubsystem && demoSubsystem->IsPlayingReplay())
	{
		TrySetViewTargetToPlayer();

		if (demoSubsystem->CaptureFrames())
		{
			if (!demoSubsystem->GetCaptureFolder())
			{
				UE_LOG(LogExperiment, Error, TEXT("Frame capture requested but no capture folder is set. Frames will not be captured."));
				return;
			}
			SetFrameCaptureFolder(*(demoSubsystem->GetCaptureFolder()));
			fps = demoSubsystem->GetCaptureFPS();
			currentTime = -1.0;
			timeStep = 1.0 / fps;
			delay = 5.0;
			worldSettings = GetWorldSettings();

			FixTimeStep(true);

			if (!worldSettings)
			{
				UE_LOG(LogExperiment, Log, TEXT("World settings is null pointer"));
				return;
			}
			if (!PlayerState)
			{
				UE_LOG(LogExperiment, Log, TEXT("No player state. Using the reference pawn's."));
				if (referencePawn && referencePawn->GetPlayerState())
					PlayerState = referencePawn->GetPlayerState();
				else
					UE_LOG(LogExperiment, Log, TEXT("Reference pawn does not have a player state either. TTLs will not be read out."));
			}
			if (!worldSettings->GetPauserPlayerState())
				worldSettings->SetPauserPlayerState(PlayerState);
			demoNetDriver = GetWorld()->GetDemoNetDriver();

			captureFrames = true;
		}
	}
}


void AExperimentSpectatorController::BeginPlayingState()
{
	Super::BeginPlayingState();
	UE_LOG(LogExperiment, Log, TEXT("Experiment spectator controller BeginPlayingState"));
	TrySetViewTargetToPlayer();
}


APawn* AExperimentSpectatorController::FindTaggedPlayerPawn()
{
	UE_LOG(LogExperiment, Log, TEXT("Spectator controller trying to find a tagged player pawn"));
	APawn* pawn = nullptr;
	for (TActorIterator<APawn> pawnIterator(GetWorld()); pawnIterator; ++pawnIterator)
	{
		if (pawnIterator->ActorHasTag(ExperimentPlayerTag))
		{
			UE_LOG(LogExperiment, Log, TEXT("Player tag found, actor is %s"), *pawnIterator->GetName());
			pawn = *pawnIterator;
			break;
		}
	}
	return pawn;
}


void AExperimentSpectatorController::TrySetViewTargetToPlayer()
{
	UE_LOG(LogExperiment, Log, TEXT("Trying to set view target to player"));
	if (GetWorld() == nullptr)
	{
		needReference = true;
		return;
	}

	UGameInstance* gameInstance = GetGameInstance();
	if (!gameInstance)
	{
		UE_LOG(LogExperiment, Log, TEXT("Has world but not game instance"));
		return;
	}

	demoSubsystem = gameInstance->GetSubsystem<UExperimentDemoSubsystem>();
	if (!demoSubsystem)
	{
		UE_LOG(LogExperiment, Error, TEXT("Could not find demo subsystem"));
		return;
	}

	APawn* playerPawn = demoSubsystem->GetPlayerPawn();
	if (playerPawn == nullptr)
	{
		UE_LOG(LogExperiment, Log, TEXT("No player pawn found"));
		return;
	}

	UE_LOG(LogExperiment, Log, TEXT("Player pawn is %s"), *playerPawn->GetName());
	this->AttachToActor(playerPawn, FAttachmentTransformRules::SnapToTargetIncludingScale);
	// Render the on-screen replay view through the pawn's own camera
	SetViewTarget(playerPawn);
	this->referencePawn = playerPawn;
	this->referencePlayerState = Cast<AExperimentSubjectState>(referencePawn->GetPlayerState());
	if (!referencePlayerState)
		UE_LOG(LogExperiment, Log, TEXT("Reference pawn player state is a null pointer"));
	UE_LOG(LogExperiment, Log, TEXT("View target set to %s"), *playerPawn->GetName());
	needReference = false;

	if (demoSubsystem->IsPlayingReplay())
	{
		if (cameras.Num() > 0)
			cameras.Empty();
		CreateCameras();
		UE_LOG(LogExperiment, Log, TEXT("Camera list initialized with %d cameras."), cameras.Num());

		// Tag the level for semantic segmentation only when a camera asks for it.
		for (AExperimentFrameCaptureCamera* camera : cameras)
		{
			if (camera && camera->GetPostProcessEffect() == EExperimentPostProcessEffect::SemanticSegmentation)
			{
				ConfigureSemanticLabels();
				semSegTagger = NewObject<USemSegTagger>(this);
				semSegTagger->SetLabelMap(semanticLabels);
				semSegTagger->SetTagLabelMap(semanticTagLabels);
				semSegTagger->SetDefaultLabel(defaultSemanticLabel);
				semSegTagger->TagActorsInLevel(*GetWorld());
				semSegTagger->RegisterSpawnHandler(GetWorld());
				break;
			}
		}

		for (int i = 0; i < frameStates.Num(); i++)
			delete frameStates[i];
		frameStates.Empty();
	}

	TArray<AActor*> attachedActors;
	playerPawn->GetAttachedActors(attachedActors);
	for (int i = 0; i < attachedActors.Num(); i++)
		UE_LOG(LogExperiment, Log, TEXT("Found attached actor %s"), *attachedActors[i]->GetName());
}


void AExperimentSpectatorController::CreateCameras()
{
	UExperimentSettingsSubsystem* settingsSubsystem = GetGameInstance()->GetSubsystem<UExperimentSettingsSubsystem>();
	if (!settingsSubsystem)
		return;

	for (const FExperimentCameraDescription& description : settingsSubsystem->GetSettings().Cameras)
	{
		AExperimentFrameCaptureCamera* camera = GetWorld()->SpawnActor<AExperimentFrameCaptureCamera>();
		camera->CameraName = description.Name;
		camera->SetImageSize(description.ImageSizeX, description.ImageSizeY);
		camera->SetPostProcessEffect(description.PostProcessing);
		camera->SetFOVAngle(description.FOV);
		AttachCamera(camera);
		AddCamera(camera);
	}
}


void AExperimentSpectatorController::AttachCamera(AExperimentFrameCaptureCamera* camera)
{
	camera->AttachToActor(referencePawn);
}


void AExperimentSpectatorController::CaptureCameras()
{
	for (int i = 0; i < cameras.Num(); i++)
		cameras[i]->SaveNextFrame();
}


void AExperimentSpectatorController::AddExperimentState(ExperimentState* state)
{
	frameStates.Add(state);
}


void AExperimentSpectatorController::Tick(float deltaTime)
{
	// The recorded player pawn streams in from the demo after this controller
	// is created, so the view binding is retried here until the pawn exists.
	if (demoSubsystem && demoSubsystem->IsPlayingReplay())
	{
		if (!replayHUD && replayHUDClass)
		{
			replayHUD = CreateWidget<UUserWidget>(this, replayHUDClass);
			if (replayHUD)
				replayHUD->AddToViewport();
		}

		if (!referencePawn)
		{
			if (demoSubsystem->GetPlayerPawn())
				TrySetViewTargetToPlayer();
		}
		else if (GetViewTarget() != referencePawn)
		{
			SetViewTarget(referencePawn);
		}

		// End of a watched replay: stop the demo and restart the level so the
		// game returns to normal play. The frame-capture path below has its
		// own end detection.
		if (!captureFrames && !stopDemo)
		{
			if (!demoNetDriver)
				demoNetDriver = GetWorld()->GetDemoNetDriver();
			if (demoNetDriver && demoNetDriver->GetDemoTotalTime() > 0 &&
				demoNetDriver->GetDemoCurrentTime() >= demoNetDriver->GetDemoTotalTime())
			{
				UE_LOG(LogExperiment, Log, TEXT("Replay playback reached its end"));
				demoSubsystem->OnReplayEnded();
				stopDemo = true;
			}
		}
	}

	if (captureFrames)
	{
		if (delay > 0)
		{
			if (!worldSettings->GetPauserPlayerState())
				worldSettings->SetPauserPlayerState(PlayerState);
			delay -= deltaTime;

			if (referencePawn && referencePawn->GetPlayerState())
			{
				UE_LOG(LogExperiment, Log, TEXT("Player state found during initial ticks"));
				referencePlayerState = Cast<AExperimentSubjectState>(referencePawn->GetPlayerState());
			}
		}
		else
		{
			if (worldSettings->GetPauserPlayerState())
			{
				worldSettings->SetPauserPlayerState(nullptr);
				UE_LOG(LogExperiment, Log, TEXT("Begin rendering frames"));
				frameNumber = 0;
			}

			if (referencePlayerState)
			{
				// if you need to do something here with the player state
			}
			else
			{
				UE_LOG(LogExperiment, Log, TEXT("Time %f, dt %f, no player state available"), demoNetDriver->GetDemoCurrentTime(), deltaTime);
			}

			CaptureCameras();
			if (referencePlayerState)
				LogExperimentState();

			if (currentTime == demoNetDriver->GetDemoCurrentTime())
			{
				captureFrames = false;
				FixTimeStep(false);
				if (referencePlayerState)
					SaveExperimentLog();
				UE_LOG(LogExperiment, Log, TEXT("End replay and end rendering frames"));
				demoSubsystem->OnReplayEnded();
				stopDemo = true;
			}
			else
			{
				currentTime = demoNetDriver->GetDemoCurrentTime();
			}

			frameNumber++;
		}
	}
	if (stopDemo && !levelReloadRequested)
	{
		// The travel takes a frame or more; run this only once, or the next
		// tick calls StopDemo() on the destroyed driver.
		levelReloadRequested = true;
		// The fixed time step is engine-global and would survive into the
		// reloaded gameplay world.
		FixTimeStep(false);
		demoNetDriver->StopDemo();
		GetWorld()->DestroyDemoNetDriver();
		RestartLevel();
	}

	Super::Tick(deltaTime);
}


double AExperimentSpectatorController::GetDemoCurrentTime() const
{
	// demoNetDriver is only cached on the frame-capture path; when just
	// watching a replay, read the driver from the world.
	UDemoNetDriver* driver = demoNetDriver ? demoNetDriver : GetWorld()->GetDemoNetDriver();
	return driver ? driver->GetDemoCurrentTime() : 0.0;
}


TArray<const UExperimentAgentComponent*> AExperimentSpectatorController::GetAgents() const
{
	TArray<const UExperimentAgentComponent*> agents;
	for (TActorIterator<AActor> actorIterator(GetWorld()); actorIterator; ++actorIterator)
	{
		actorIterator->ForEachComponent<UExperimentAgentComponent>(false,
			[&agents](const UExperimentAgentComponent* component)
			{
				if (component->IsAgentRegistered())
					agents.Add(component);
			});
	}
	return agents;
}


void AExperimentSpectatorController::SaveExperimentLog()
{
#define FORMAT_TO_CHAR_ARRAY(string, ...) TCHAR_TO_ANSI(*FString::Printf(TEXT(string), ##__VA_ARGS__))

	if (!demoSubsystem->GetDemoName())
	{
		UE_LOG(LogExperiment, Error, TEXT("No demo name is set. The experiment log will not be saved."));
		return;
	}

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

	Experiment_WRITE_INDENTS(logFile, indentation);
	logFile << Experiment_WRITE_SINGLE_TAG("Experiment-Type",
		TCHAR_TO_ANSI(*UExperimentType::ToString(experimentType))) << endl;

	Experiment_WRITE_INDENTS(logFile, indentation);
	logFile << Experiment_WRITE_SINGLE_TAG("Player-ID", playerAgentID) << endl;

	Experiment_WRITE_INDENTS(logFile, indentation);
	logFile << Experiment_WRITE_SINGLE_TAG("Capture-FPS", demoSubsystem->GetCaptureFPS()) << endl;

	Experiment_WRITE_INDENTS(logFile, indentation);
	logFile << Experiment_WRITE_SINGLE_TAG("Seconds-To-Run-Start", referencePlayerState->GetSecondsToStartOfRun()) << endl;

	// Per-frame states
	if (frameStates.Num() > 0)
	{
		UE_LOG(LogExperiment, Log, TEXT("%d frames to save"), frameStates.Num());
		int percentStep = frameStates.Num() / 100;
		percentStep = percentStep ? percentStep : 1;

		for (int frame = 0; frame < frameStates.Num(); frame++)
		{
			frameStates[frame]->WriteToFile(logFile, indentation);

			if (frame % percentStep == 0)
				UE_LOG(LogExperiment, Log, TEXT("%d/%d frames written"), frame + 1, frameStates.Num() + 1);
		}
	}

	indentation--;
	logFile << "</Experiment>" << endl;
	logFile.close();
	UE_LOG(LogExperiment, Log, TEXT("File saved to %s"), *fileName);

	if (demoSubsystem->IsAutoRender())
		FGenericPlatformMisc::RequestExit(false);
}


void AExperimentSpectatorController::SetFrameCaptureFolder(const FString& folder)
{
	// Each camera writes into its own subfolder so their frames do not collide.
	for (int i = 0; i < cameras.Num(); i++)
		cameras[i]->SetSaveFolder(FPaths::Combine(folder, cameras[i]->CameraName));
}


void AExperimentSpectatorController::FixTimeStep(bool fix)
{
	FApp::SetFixedDeltaTime(fix ? timeStep : 0);
	FApp::SetBenchmarking(fix);
	FApp::SetUseFixedTimeStep(fix);
}


void AExperimentSpectatorController::SpawnLoggerComponent()
{
	UGameInstance* gameInstance = GetWorld()->GetGameInstance();
	demoSubsystem = gameInstance->GetSubsystem<UExperimentDemoSubsystem>();

	if (demoSubsystem)
		experimentType = demoSubsystem->GetExperimentType();
	else
		experimentType = EExperimentType::Other;

	UE_LOG(LogExperiment, Log, TEXT("Base Experiment logger component spawned"));
	loggerComponent = NewObject<UExperimentLoggerComponent>(this);
	loggerComponent->SetParentController(this);
}


void AExperimentSpectatorController::LogExperimentState()
{
	if (!loggerComponent)
		SpawnLoggerComponent();
	loggerComponent->LogFrame();
}


void AExperimentSpectatorController::SetReferencePawn(APawn* newReferencePawn)
{
	this->referencePawn = newReferencePawn;
}
