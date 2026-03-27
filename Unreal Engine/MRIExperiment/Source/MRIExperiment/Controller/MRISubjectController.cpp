// Copyright (c) Gallant Lab. All Rights Reserved.

#include "MRIExperiment.h"
#include "Controller/MRISubjectController.h"
#include "Settings/MRISettings.h"
#include "Demo/MRIDemoSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "EngineUtils.h"

#define BOOL_TO_TEXT(x) x ? TEXT("yes") : TEXT("no")


/**
 * Constructs the controller, initializes the random stream, enables ticking,
 * and locates the 440 Hz synchronization beep sound cue asset.
 */
AMRISubjectController::AMRISubjectController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	randomStream = FRandomStream();
	PrimaryActorTick.bCanEverTick = true;

	// Find the beep sound cue
	static ConstructorHelpers::FObjectFinder<USoundCue> beepCueFinder(TEXT("/Game/Sounds/440Hz_Cue"));
	beepCue = beepCueFinder.Object;
	UE_LOG(LogMRI, Log, TEXT("440Hz beep pointer is%s valid"), beepCue->IsValidLowLevelFast() ? TEXT("") : TEXT(" not"));
}


/**
 * Reads settings from MRISettings, configures auto-eyetracking, auto-demo-recording,
 * auto-stop thresholds, and applies the display resolution if not yet applied.
 * Also triggers auto-rendering of all replays if RenderAll is set.
 */
void AMRISubjectController::BeginPlay()
{
	Super::BeginPlay();

	FindDemoSubsystem();
	bShowMouseCursor = false;

	UMRISettings& settings = demoSubsystem->GetSettings();

	TTLsToExperimentStart = settings.TTLsBeforeExperimentStart;

	autoEyetrack = settings.AutoEyetrackingCalibration;
	if (!autoEyetrack)
		eyetrackingState = 2;

	autoTriggerDemo = settings.AutoTriggerDemoRecording;

	UE_LOG(LogMRI, Log, TEXT("MRI controller auto eyetrack %s auto demo record %s"), BOOL_TO_TEXT(autoEyetrack), BOOL_TO_TEXT(autoTriggerDemo));

	autoStopDemoLimit = settings.SecondsToDemoStop;
	bAutoStopDemo = autoStopDemoLimit > 0;

	if (!demoSubsystem->HasResolutionBeenSet())
	{
		ConsoleCommand(*FString("r.setRes ").Append(settings.Resolution));
		demoSubsystem->UpdateResolutionHasBeenSet();
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


/**
 * Caches a typed pointer to AMRISubjectState from PlayerState and configures
 * the net update frequency for reliable replication.
 */
void AMRISubjectController::Possess(APawn* pawn)
{
	Super::Possess(pawn);
	subjectState = Cast<AMRISubjectState>(PlayerState);
	check(subjectState != nullptr);
	subjectState->NetUpdateFrequency = 30;
	subjectState->MinNetUpdateFrequency = 30;
}


/**
 * Attempts to find UMRIDemoSubsystem on the current game instance.
 * @return true if the subsystem was found and cached.
 */
bool AMRISubjectController::FindDemoSubsystem()
{
	UE_LOG(LogMRI, Log, TEXT("Trying to find demo subsystem"));
	UGameInstance* gameInstance = GetWorld()->GetGameInstance();
	demoSubsystem = gameInstance->GetSubsystem<UMRIDemoSubsystem>();
	if (demoSubsystem)
	{
		UE_LOG(LogMRI, Log, TEXT("MRI controller found demo subsystem"));
		return true;
	}
	UE_LOG(LogMRI, Error, TEXT("MRI controller failed to find demo subsystem"));
	return false;
}


/**
 * Per-frame update: refreshes demo state, runs ExperimentTick after enough TTLs,
 * decrements the display text countdown, and auto-stops the demo if idle too long.
 */
void AMRISubjectController::Tick(float dTime)
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
			UE_LOG(LogMRI, Log, TEXT("MRI subject controller stopping demo after %.2f seconds without a TTL after %d TTLs"), secondsWithoutTTL, subjectState->GetTotalTTLs());
		}
}


/**
 * Binds TTL press/release and beep press/release actions from the input component.
 */
void AMRISubjectController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent)
	{
		// TTL info
		InputComponent->BindAction("TTL", IE_Pressed, this, &AMRISubjectController::TTLdown);
		InputComponent->BindAction("TTL", IE_Released, this, &AMRISubjectController::TTLup);

		// Beep info - ephys
		InputComponent->BindAction("PlayBeep", IE_Pressed, this, &AMRISubjectController::PlayBeep);
		InputComponent->BindAction("PlayBeep", IE_Released, this, &AMRISubjectController::TTLup);
	}
}


/**
 * Handles the falling edge of a TTL pulse. Records the TTL in subject state,
 * increments the counter, resets the no-TTL timer, and on the first TTL of
 * a run may auto-start demo recording and record the time since demo start.
 */
void AMRISubjectController::TTLdown()
{
	UE_LOG(LogMRI, Log, TEXT("TTL on %s"), *FDateTime::Now().ToString());
	subjectState->TTL = true;
	subjectState->TotalTTLs++;
	secondsWithoutTTL = 0.0;

	if (!demoSubsystem)
		FindDemoSubsystem();
	if (!demoSubsystem)
		UE_LOG(LogMRI, Error, TEXT("Demo subsystem finding for TTL down failed"));

	FTimespan elapsed = FDateTime::Now() - demoSubsystem->demoStartTime;
	UE_LOG(LogMRI, Log, TEXT("%02d:%02d:%02d.%03d since demo start"), elapsed.GetHours(), elapsed.GetMinutes(), elapsed.GetSeconds(), elapsed.GetFractionMilli());

	if (isFirstTTLInRun)
	{
		if (autoTriggerDemo)
		{
			if (!demoSubsystem)
			{
				UE_LOG(LogMRI, Error, TEXT("No demo subsystem; did not auto trigger demo start"));
			}
			else
			{
				if (!demoSubsystem->IsRecordingReplay())
				{
					demoSubsystem->StartRecordingReplay(FString(), FString());
					UE_LOG(LogMRI, Log, TEXT("Demo recording auto-triggered by first TTL"));
					elapsed = FDateTime::Now() - demoSubsystem->demoStartTime;
				}
			}
		}

		SetSecondsToStartOfRun(elapsed.GetTotalSeconds());
		isFirstTTLInRun = false;
	}
}


/**
 * Handles the rising edge of a TTL pulse. Clears the TTL flag in subject state,
 * optionally triggers eyetracking calibration, and decrements TTLsToExperimentStart.
 */
void AMRISubjectController::TTLup()
{
	UE_LOG(LogMRI, Log, TEXT("TTL off %s"), *FDateTime::Now().ToString());
	subjectState->TTL = false;

	if (autoEyetrack && !(eyetrackingState))
	{
		UE_LOG(LogMRI, Log, TEXT("Auto eyetracking calibration would trigger here"));
		eyetrackingState = 1;
	}

	if (TTLsToExperimentStart && (eyetrackingState == 2))
		TTLsToExperimentStart--;

	UE_LOG(LogMRI, Log, TEXT("Current eyetracking state %d"), eyetrackingState);

	if (TTLsToExperimentStart)
		UE_LOG(LogMRI, Log, TEXT("%d more TTLs to experiment start"), TTLsToExperimentStart);
}


/** Returns whether the TTL pulse is currently active by querying subject state. */
bool AMRISubjectController::isTTL() const
{
	return subjectState->isTTL();
}


/**
 * Plays the 440 Hz synchronization beep sound, sets the PlayBeep flag in subject state,
 * starts a 0.5-second timer to clear the flag, and also triggers TTLdown for combined
 * ephys/fMRI experiments.
 */
void AMRISubjectController::PlayBeep()
{
	if (beepCue)
	{
		UE_LOG(LogMRI, Log, TEXT("Playing beep"));
		UGameplayStatics::PlaySound2D(GetWorld(), beepCue);
		subjectState->PlayBeep = true;
		GetWorld()->GetTimerManager().SetTimer(beepTimer, this, &AMRISubjectController::OnBeepEnd, 0.5, false);
		TTLdown();	// also include the TTL marker for this
	}
}


/**
 * Records the time from demo start to the first TTL pulse and marks the run as started.
 */
void AMRISubjectController::SetSecondsToStartOfRun(float seconds)
{
	UE_LOG(LogMRI, Log, TEXT("First TTL at time %f seconds"), seconds);
	subjectState->secondsToStartOfRun = seconds;
	isFirstTTLInRun = false;
}


/**
 * Queries the demo subsystem for the current recording status and caches it.
 * Called each tick before experiment logic runs.
 */
void AMRISubjectController::UpdateDemoState()
{
	if (!demoSubsystem)
		FindDemoSubsystem();
	if (!demoSubsystem)
	{
		UE_LOG(LogMRI, Error, TEXT("Demo subsystem finding failed, demo state not updated"));
		return;
	}
	currentDemoState = demoSubsystem->GetDemoState();
}


/**
 * Returns true on the first tick after demo recording transitions from recording to stopped.
 * Uses lastDemoState/currentDemoState to detect the transition edge.
 */
bool AMRISubjectController::IsDemoEnded()
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
				UE_LOG(LogMRI, Log, TEXT("Demo stop detected"));
				lastDemoState = 2;
				return true;
			}
			else return false;
		default:
			UE_LOG(LogMRI, Error, TEXT("Supposedly unreachable code reached"));
			return false;
	}
}


/**
 * Resets all run-level experiment state: demo state tracking, timers, eyetracking state,
 * TTL countdown, no-TTL timer, and subject state.
 */
void AMRISubjectController::ResetExperimentState()
{
	lastDemoState = 0;
	currentDemoState = 0;
	timeSinceDemoStart = 0.0;
	eyetrackingState = autoEyetrack ? 0 : 2;
	isFirstTTLInRun = true;
	TTLsToExperimentStart = demoSubsystem->GetSettings().TTLsBeforeExperimentStart;
	secondsWithoutTTL = 0;
	subjectState->ResetExperimentState();
}


/**
 * Increments the subject's point total by increment and fires SetDisplayPointsDelegate.
 */
void AMRISubjectController::UpdatePoints(int increment)
{
	subjectState->Points += increment;
	SetDisplayPointsDelegate.ExecuteIfBound(subjectState->GetPoints());
}


/**
 * Sets the on-screen display text and color, records the prompt type in subject state,
 * and starts the display countdown timer.
 */
void AMRISubjectController::SetDisplayText(const FString& text, double duration, EDisplayedPromptType displayedPromptType, EDisplayTextColor color)
{
	SetDisplayTextDelegate.ExecuteIfBound(text);
	SetDisplayTextColorDelegate.ExecuteIfBound(color);
	subjectState->displayedPromptType = displayedPromptType;
	displayTextTimeRemaining = duration;
}


/** Overload that accepts prompt type before duration; delegates to the primary overload. */
void AMRISubjectController::SetDisplayText(const FString& text, EDisplayedPromptType displayedPromptType, double duration, EDisplayTextColor color)
{
	SetDisplayText(text, duration, displayedPromptType, color);
}


/**
 * Clears the on-screen text by firing an empty string delegate, resets the text color,
 * and calls OnEndDisplayText to notify subclasses.
 */
void AMRISubjectController::ClearDisplayText()
{
	SetDisplayTextDelegate.ExecuteIfBound(EMPTY_STRING);
	SetDisplayTextColorDelegate.ExecuteIfBound(EDisplayTextColor::White);
	OnEndDisplayText();
}


/** Resets the displayed prompt type to None in subject state. Override to do more. */
void AMRISubjectController::OnEndDisplayText()
{
	subjectState->displayedPromptType = EDisplayedPromptType::None;
}


/** Returns the cached demo subsystem, re-querying if the cache is null. */
UMRIDemoSubsystem* AMRISubjectController::GetDemoSubsystem()
{
	if (!demoSubsystem)
		FindDemoSubsystem();
	return demoSubsystem;
}
