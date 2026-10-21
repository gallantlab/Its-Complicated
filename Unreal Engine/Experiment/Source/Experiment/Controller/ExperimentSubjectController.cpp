// Copyright (c) Gallant Lab. All Rights Reserved.

#include "ExperimentSubjectController.h"

#include "Settings/ExperimentSettingsSubsystem.h"
#include "Demo/ExperimentDemoSubsystem.h"

#include "EnhancedInputComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#define BOOL_TO_TEXT(x) x ? TEXT("yes") : TEXT("no")


AExperimentSubjectController::AExperimentSubjectController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	randomStream = FRandomStream();
	PrimaryActorTick.bCanEverTick = true;
}


void AExperimentSubjectController::BeginPlay()
{
	Super::BeginPlay();

	FindDemoSubsystem();
	bShowMouseCursor = false;

	settingsSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UExperimentSettingsSubsystem>();
	UExperimentSettings& settings = settingsSubsystem->GetSettings();
	demoSubsystem->ReadSettings(&settings);

	TTLsToExperimentStart = settings.TTLsBeforeExperimentStart;

	autoEyetrack = settings.AutoEyetrackingCalibration;
	if (!autoEyetrack)
		eyetrackingState = 2;

	autoTriggerDemo = settings.AutoTriggerDemoRecording;

	UE_LOG(LogExperiment, Log, TEXT("Experiment controller auto eyetrack %s auto demo record %s"), BOOL_TO_TEXT(autoEyetrack), BOOL_TO_TEXT(autoTriggerDemo));

	autoStopDemoLimit = settings.SecondsToDemoStop;
	bAutoStopDemo = autoStopDemoLimit > 0;

	if (!settingsSubsystem->HasResolutionBeenSet())
	{
		ConsoleCommand(*FString("r.setRes ").Append(settings.Resolution));
		settingsSubsystem->UpdateResolutionHasBeenSet();
	}

	// If auto-rendering has been specified, force subsystem to enumerate replays
	// because that triggers the next auto rendering.
	// This will continue until all has been rendered because a new controller
	// is spawned after each demo replay finishes.
	if (settings.RenderAll)
	{
		demoSubsystem->FindReplays();
		if (demoSubsystem->replayToRender.Len() > 0)
			demoSubsystem->DemoRenderFrames(demoSubsystem->replayToRender, settings.RenderAll);
	}
}


void AExperimentSubjectController::OnPossess(APawn* pawn)
{
	Super::OnPossess(pawn);
	subjectState = Cast<AExperimentSubjectState>(PlayerState);
	check(subjectState != nullptr);
	subjectState->SetNetUpdateFrequency(30);
	subjectState->SetMinNetUpdateFrequency(30);
	pawn->Tags.Add(ExperimentPlayerTag);
}


bool AExperimentSubjectController::FindDemoSubsystem()
{
	UE_LOG(LogExperiment, Log, TEXT("Trying to find demo subsystem"));
	UGameInstance* gameInstance = GetWorld()->GetGameInstance();
	demoSubsystem = gameInstance->GetSubsystem<UExperimentDemoSubsystem>();
	if (demoSubsystem)
	{
		UE_LOG(LogExperiment, Log, TEXT("Experiment controller found demo subsystem"));
		return true;
	}
	UE_LOG(LogExperiment, Error, TEXT("Experiment controller failed to find demo subsystem"));
	return false;
}


void AExperimentSubjectController::Tick(float dTime)
{
	Super::Tick(dTime);

	UpdateDemoState();
	if (currentDemoState == 1)	// count time since demo started if demo is recording
		timeSinceDemoStart += dTime;

	if (!TTLsToExperimentStart)
	{
		ExperimentTick(dTime);
		if (IsDemoEnded())	// reset experiment data when demo recording ended for this run
			ResetExperimentState();
	}

	if (displayTextTimeRemaining > 0)
	{
		displayTextTimeRemaining -= dTime;
		if (displayTextTimeRemaining <= 0)
		{
			ClearDisplayText();
		}
	}

	secondsWithoutTTL += dTime;
	if (bAutoStopDemo && !isFirstTTLInRun)
		if (secondsWithoutTTL > autoStopDemoLimit)
		{
			demoSubsystem->StopRecordingReplay();
			UE_LOG(LogExperiment, Log, TEXT("Experiment subject controller stopping demo after %.2f seconds without a TTL after %d TTLs"), secondsWithoutTTL, subjectState->GetTotalTTLs());
		}
}


void AExperimentSubjectController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent)
	{
		UEnhancedInputComponent* enhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
		if (TTLAction && enhancedInput)
		{
			enhancedInput->BindAction(TTLAction, ETriggerEvent::Started, this, &AExperimentSubjectController::TTLdown);
			enhancedInput->BindAction(TTLAction, ETriggerEvent::Completed, this, &AExperimentSubjectController::TTLup);
		}
		else
		{
			InputComponent->BindAction("TTL", IE_Pressed, this, &AExperimentSubjectController::TTLdown);
			InputComponent->BindAction("TTL", IE_Released, this, &AExperimentSubjectController::TTLup);
		}
	}
}


void AExperimentSubjectController::TTLdown()
{
	UE_LOG(LogExperiment, Log, TEXT("TTL on %s"), *FDateTime::Now().ToString());
	subjectState->TTL = true;
	subjectState->TotalTTLs++;
	secondsWithoutTTL = 0.0;

	if (!demoSubsystem)
		FindDemoSubsystem();
	if (!demoSubsystem)
	{
		UE_LOG(LogExperiment, Error, TEXT("Demo subsystem finding for TTL down failed"));
		return;
	}

	FTimespan elapsed = FDateTime::Now() - demoSubsystem->demoStartTime;
	UE_LOG(LogExperiment, Log, TEXT("%02d:%02d:%02d.%03d since demo start"), elapsed.GetHours(), elapsed.GetMinutes(), elapsed.GetSeconds(), elapsed.GetFractionMilli());

	if (isFirstTTLInRun)
	{
		if (autoTriggerDemo)
		{
			if (!demoSubsystem)
			{
				UE_LOG(LogExperiment, Error, TEXT("No demo subsystem; did not auto trigger demo start"));
			}
			else
			{
				if (!demoSubsystem->IsRecordingReplay())
				{
					demoSubsystem->StartRecordingReplay(FString(), FString());
					UE_LOG(LogExperiment, Log, TEXT("Demo recording auto-triggered by first TTL"));
					elapsed = FDateTime::Now() - demoSubsystem->demoStartTime;
				}
			}
		}

		SetSecondsToStartOfRun(elapsed.GetTotalSeconds());
		isFirstTTLInRun = false;
	}
}


void AExperimentSubjectController::TTLup()
{
	UE_LOG(LogExperiment, Log, TEXT("TTL off %s"), *FDateTime::Now().ToString());
	subjectState->TTL = false;

	if (autoEyetrack && !(eyetrackingState))
	{
		UE_LOG(LogExperiment, Log, TEXT("Auto eyetracking calibration would trigger here"));
		eyetrackingState = 1;
	}

	if (TTLsToExperimentStart && (eyetrackingState == 2))
		TTLsToExperimentStart--;

	UE_LOG(LogExperiment, Log, TEXT("Current eyetracking state %d"), eyetrackingState);

	if (TTLsToExperimentStart)
		UE_LOG(LogExperiment, Log, TEXT("%d more TTLs to experiment start"), TTLsToExperimentStart);
}


bool AExperimentSubjectController::isTTL() const
{
	return subjectState->isTTL();
}


void AExperimentSubjectController::SetSecondsToStartOfRun(float seconds)
{
	UE_LOG(LogExperiment, Log, TEXT("First TTL at time %f seconds"), seconds);
	subjectState->secondsToStartOfRun = seconds;
	isFirstTTLInRun = false;
}


void AExperimentSubjectController::UpdateDemoState()
{
	if (!demoSubsystem)
		FindDemoSubsystem();
	if (!demoSubsystem)
	{
		UE_LOG(LogExperiment, Error, TEXT("Demo subsystem finding failed, demo state not updated"));
		return;
	}
	currentDemoState = demoSubsystem->GetDemoState();
}


bool AExperimentSubjectController::IsDemoEnded()
{
	switch (currentDemoState)
	{
		case 0:
			lastDemoState = 0;
			return false;
		case 1:
			lastDemoState = 1;
			return false;
		case 2:
			if (lastDemoState < 2)	// demo state switched into this status on last tick
			{
				UE_LOG(LogExperiment, Log, TEXT("Demo stop detected"));
				lastDemoState = 2;
				return true;
			}
			else return false;
		default:
			UE_LOG(LogExperiment, Error, TEXT("Supposedly unreachable code reached"));
			return false;
	}
}


void AExperimentSubjectController::ResetExperimentState()
{
	lastDemoState = 0;
	currentDemoState = 0;
	timeSinceDemoStart = 0.0;
	eyetrackingState = autoEyetrack ? 0 : 2;
	isFirstTTLInRun = true;
	TTLsToExperimentStart = settingsSubsystem->GetSettings().TTLsBeforeExperimentStart;
	secondsWithoutTTL = 0;
	subjectState->ResetExperimentState();
}


void AExperimentSubjectController::UpdatePoints(int increment)
{
	subjectState->Points += increment;
	SetDisplayPointsDelegate.ExecuteIfBound(subjectState->GetPoints());
}


void AExperimentSubjectController::SetDisplayText(const FString& text, double duration, EDisplayedPromptType displayedPromptType, EDisplayTextColor color)
{
	SetDisplayTextDelegate.ExecuteIfBound(text);
	SetDisplayTextColorDelegate.ExecuteIfBound(color);
	subjectState->displayedPromptType = displayedPromptType;
	displayTextTimeRemaining = duration;
}


void AExperimentSubjectController::SetDisplayText(const FString& text, EDisplayedPromptType displayedPromptType, double duration, EDisplayTextColor color)
{
	SetDisplayText(text, duration, displayedPromptType, color);
}


void AExperimentSubjectController::ClearDisplayText()
{
	SetDisplayTextDelegate.ExecuteIfBound(EMPTY_STRING);
	SetDisplayTextColorDelegate.ExecuteIfBound(EDisplayTextColor::White);
	OnEndDisplayText();
}


void AExperimentSubjectController::OnEndDisplayText()
{
	subjectState->displayedPromptType = EDisplayedPromptType::None;
}


UExperimentDemoSubsystem* AExperimentSubjectController::GetDemoSubsystem()
{
	if (!demoSubsystem)
		FindDemoSubsystem();
	return demoSubsystem;
}
