// Copyright (c) Gallant Lab. All Rights Reserved.

#pragma once

#include "MRIExperiment.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "State/MRISubjectState.h"

#include "MRISubjectController.generated.h"

class UMRIDemoSubsystem;
class UMRISettings;
class USoundCue;

const FString EMPTY_STRING = FString("");

/// Display text color
UENUM(BlueprintType)
enum class EDisplayTextColor : uint8
{
	White	UMETA(DisplayName = "White"),
	Red		UMETA(DisplayName = "Red"),
};

// Delegates for displaying things
DECLARE_DELEGATE_OneParam(FSetDisplayTextDelegate, FString);

DECLARE_DELEGATE_OneParam(FSetDisplayTextColorDelegate, EDisplayTextColor);

DECLARE_DELEGATE_OneParam(FSetDisplayPointsDelegate, int);

DECLARE_DELEGATE_OneParam(FSetTimeRemainingDelegate, int);


/**
 * Base controller for subjects in MRI experiments.
 * Handles TTL pulse tracking, experiment lifecycle, demo recording management,
 * eyetracking calibration triggers, and display text for prompts and feedback.
 */
UCLASS()
class MRIEXPERIMENT_API AMRISubjectController : public APlayerController
{
	GENERATED_BODY()

public:

	AMRISubjectController(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void Possess(APawn* pawn) override;

	UFUNCTION(BlueprintCallable)
	void SetEyetrackingEnded()
	{
		eyetrackingState = 2;
	}

	// Is the game demo recording ended? If so, reset
	bool IsDemoEnded();

	// Override point for experiment logic, called each tick 
	virtual void ExperimentTick(float deltaTime) = 0;

	virtual void TTLdown();

	virtual void TTLup();

	bool isTTL() const;

	// Display delegates
	FSetDisplayTextDelegate SetDisplayTextDelegate;

	FSetDisplayTextColorDelegate SetDisplayTextColorDelegate;

	FSetDisplayPointsDelegate SetDisplayPointsDelegate;

	FSetTimeRemainingDelegate SetTimeRemainingDelegate;

protected:

	double displayTextTimeRemaining = 0;

	/**
	 * Overridden to also try to update the display text
	 * @param increment amount to add to points
	 */
	virtual void UpdatePoints(int increment = 1);

	/**
	 * Method used by all inheriting classes to set display text and color.
	 * This base class takes care of how long to show it.
	 * @param text					text to display
	 * @param duration				time in seconds to display it; -1 to display continuously
	 * @param displayedPromptType	type of prompt being displayed
	 * @param color					color of text to show
	 */
	void SetDisplayText(const FString& text, double duration = 2.0,
						EDisplayedPromptType displayedPromptType = EDisplayedPromptType::Unknown,
						EDisplayTextColor color = EDisplayTextColor::White);

	void SetDisplayText(const FString& text,
						EDisplayedPromptType displayedPromptType = EDisplayedPromptType::Unknown,
						double duration = 2.0,
						EDisplayTextColor color = EDisplayTextColor::White);

	void ClearDisplayText();

	/**
	 * Callback on end of text display. Override to do things when text finishes displaying.
	 */
	virtual void OnEndDisplayText();

	virtual void ResetExperimentState();

	virtual void SetupInputComponent() override;

	void PlayBeep();

	UFUNCTION(BlueprintCallable)
	void SetSecondsToStartOfRun(float seconds);

	bool isFirstTTLInRun = true;

	FTimerHandle beepTimer;

	void OnBeepEnd()
	{
		subjectState->PlayBeep = false;
	}

	void ResetPoints()
	{
		subjectState->Points = 0;
	}

	double timeSinceDemoStart = 0.0;

	double secondsToFirstTTL = 0.0;

	bool FindDemoSubsystem();

	UMRIDemoSubsystem* demoSubsystem = nullptr;

	UMRIDemoSubsystem* GetDemoSubsystem();

	// Eyetracking state
	// Automatically do eyetracking at beginning of run?
	bool autoEyetrack = false;
	int eyetrackingState = 0;	// 0 = not started, 1 = started, 2 = ended

	FRandomStream randomStream;

	UPROPERTY()
	AMRISubjectState* subjectState = nullptr;

private:

	int lastDemoState = 0;		// used to check for changes in demo recording state on each tick
	int currentDemoState = 0;	// used in conjunction with lastDemoState

	void UpdateDemoState();

	// Number of TTLs to wait for before starting experiment logic
	int TTLsToExperimentStart = 5;

	bool autoTriggerDemo = false;
	bool bAutoStopDemo = false;
	float autoStopDemoLimit = 0;
	float secondsWithoutTTL = 0.0;

	// Used for auto render all; wait n seconds into game init before render
	// because calling demoSubsystem->FindReplays() to autotrigger rendering only works once per launch
	// and on the second trigger, no spectator controller is spawned
	bool autoRender = false;
	double secondsUntilRenderNext = 10;

	// A reference for a sound cue to play for synchronization in ephys experiments.
	USoundCue* beepCue;
};
