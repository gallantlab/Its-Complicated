// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExperimentSpectatorController.h"
#include "ExampleSpectatorController.generated.h"

/**
 *  Spectator controller for the example experiment.
 *  During replay rendering, spawns the example logger component and attaches
 *  the settings-file cameras to the subject's gameplay camera.
 *  Each tick, reconstructs the HUD contents from the replayed world: the
 *  recorded trial boxes carry the collection progress, the recorded player
 *  state carries the points.
 */
UCLASS()
class AExampleSpectatorController : public AExperimentSpectatorController
{
	GENERATED_BODY()

public:

	/**
	 * Constructor.
	 * @param objectInitializer  Unreal object initializer forwarded to the parent class.
	 */
	AExampleSpectatorController(const FObjectInitializer& objectInitializer);

	/**
	 * Loads the replay HUD widget class. Deferred to BeginPlay because the
	 * widget references this class, which forbids a constructor-time load.
	 */
	virtual void BeginPlay() override;

	/**
	 * Runs the base replay logic, then the HUD reconstruction.
	 * @param deltaTime  Time elapsed since the last frame in seconds.
	 */
	virtual void Tick(float deltaTime) override;

	/**
	 * Returns the bearing from the replayed camera forward direction to the
	 * nearest remaining trial box. The HUD polls this to rotate its direction
	 * arrow, mirroring the live controller's function of the same name.
	 * @param angleDegrees  Yaw offset in degrees, -180 to 180; 0 = straight ahead.
	 * @return true while uncollected boxes exist in the replay.
	 */
	UFUNCTION(BlueprintPure, Category = "Example|Task")
	bool GetDirectionToNearestItem(float& angleDegrees) const;

	/**
	 * Collects all actors in the replay world tagged as trial items.
	 * @return The remaining trial boxes.
	 */
	TArray<AActor*> GetTrialItems() const;

protected:

	/**
	 * Spawns the example logger component in place of the base one.
	 */
	virtual void SpawnLoggerComponent() override;

	/**
	 * Attaches a camera to the subject pawn's gameplay camera so captured
	 * frames match what the subject saw.
	 */
	virtual void AttachCamera(AExperimentFrameCaptureCamera* camera) override;

	/**
	 * Fills the semantic label maps: character 1, trial boxes 2, sky 4,
	 * everything else the default 3.
	 */
	virtual void ConfigureSemanticLabels() override;

	/**
	 * Derives the HUD contents from the replayed world and fires the display
	 * delegates: box appearances start a trial ("Collect N items"), each
	 * disappearance is a collection ("Collected M/N", "All Collected"), the
	 * timer is demo time since the boxes appeared, and points come from the
	 * recorded player state.
	 * @param deltaTime  Time elapsed since the last frame in seconds.
	 */
	void ReconstructHUD(float deltaTime);

	/**
	 * Fires the display text delegate and arms the 2-second clear countdown,
	 * matching the live controller's display duration.
	 * @param text  Text to show.
	 */
	void ShowDisplayText(const FString& text);

	/** Boxes present on the previous tick, to detect appearances and collections. */
	int observedItemCount = 0;

	/** Box count at the current trial's start. */
	int trialItemTotal = 0;

	/** Demo time at the current trial's start, in seconds. */
	double trialStartDemoTime = 0.0;

	/** Whole seconds last reported to the HUD timer, to fire only on change. */
	int lastReportedTrialSecond = -1;

	/** Points last reported to the HUD. */
	int lastObservedPoints = 0;

	/** Seconds remaining before the display text clears. */
	double displayTextSecondsRemaining = 0.0;
};
