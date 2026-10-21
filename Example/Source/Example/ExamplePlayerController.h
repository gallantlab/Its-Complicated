// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExperimentSubjectController.h"
#include "ExamplePlayerController.generated.h"

class AStaticMeshActor;
class UInputMappingContext;
class UUserWidget;

/**
 *  Subject controller for the example experiment.
 *  Derives from AExperimentSubjectController, which supplies the TTL tracking and
 *  demo recording lifecycle, and manages the third person template's
 *  Enhanced Input mappings.
 */
UCLASS()
class AExamplePlayerController : public AExperimentSubjectController
{
	GENERATED_BODY()

public:

	/**
	 * Constructor.
	 * @param ObjectInitializer  Unreal object initializer forwarded to the parent class.
	 */
	AExamplePlayerController(const FObjectInitializer& ObjectInitializer);

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** HUD widget class created on BeginPlay */
	UPROPERTY(EditAnywhere, Category = "Example|HUD")
	TSubclassOf<UUserWidget> GameHUDClass;

	/** Settings menu widget class opened with the Escape key */
	UPROPERTY(EditAnywhere, Category = "Example|HUD")
	TSubclassOf<UUserWidget> SettingsHUDClass;

	/** The settings menu widget. Created on first open, reused afterwards. */
	UPROPERTY()
	TObjectPtr<UUserWidget> SettingsHUDWidget;

	/** Opens the settings menu unless it is already on screen. Bound to Escape. */
	void HandleEscapePressed();

	/** Adds the settings menu to the viewport, shows the cursor, and routes input to the UI.
	 *  The menu's Accept button reverses all three when it closes. */
	void OpenSettingsMenu();

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/**
	 * Per-frame experiment logic. Called by the base controller every frame
	 * once the configured number of TTLs has been received.
	 * Runs the collection task: waits out the inter-trial pause, starts trials,
	 * reports the elapsed trial time, and collects items the pawn overlaps.
	 * @param deltaTime  Time elapsed since the last frame in seconds.
	 */
	virtual void ExperimentTick(float deltaTime) override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

public:

	/**
	 * Returns the bearing from the camera forward direction to the nearest
	 * uncollected item. The HUD polls this to rotate its direction arrow.
	 * @param angleDegrees  Yaw offset in degrees, -180 to 180; 0 = straight ahead.
	 * @return true while a trial with uncollected items is running.
	 */
	UFUNCTION(BlueprintPure, Category = "Example|Task")
	bool GetDirectionToNearestItem(float& angleDegrees) const;

protected:

	/**
	 * Resets the collection task alongside the base experiment state: destroys
	 * any remaining items and returns to the inter-trial pause with no delay.
	 * The base controller calls this when a run ends (e.g. demo recording stopped).
	 */
	virtual void ResetExperimentState() override;

	/**
	 * Starts a new trial: picks a random item count (1-3), spawns the items at
	 * random reachable points on the navmesh, and shows the "Collect N items" prompt.
	 */
	void StartTrial();

	/**
	 * Collects the item at the given index: destroys it, awards a point, and
	 * shows either the progress prompt or, on the last item, the completion
	 * prompt and the inter-trial pause.
	 * @param itemIndex  Index into TrialItems.
	 */
	void CollectItem(int itemIndex);

	/**
	 * Spawns one collectible box at the given navmesh location.
	 * @param location  World-space point on the navmesh.
	 * @return The spawned actor, or nullptr on failure.
	 */
	AStaticMeshActor* SpawnItem(const FVector& location);

	/** Class of the collectible boxes, the BP_TrialItem Blueprint. Its class
	 *  defaults carry the mesh, collision, tag, and replication, so replayed
	 *  instances are complete without replicating any properties. */
	UPROPERTY(EditAnywhere, Category = "Example|Task")
	TSubclassOf<AStaticMeshActor> ItemClass;

	/** Items of the current trial that have not been collected yet. */
	UPROPERTY()
	TArray<TObjectPtr<AStaticMeshActor>> TrialItems;

	/** Number of items spawned for the current trial. */
	int ItemsToCollect = 0;

	/** Number of items collected so far in the current trial. */
	int ItemsCollected = 0;

	/** Seconds since the current trial started. */
	double TrialElapsedSeconds = 0.0;

	/** Whole seconds last reported to the HUD timer, to fire only on change. */
	int LastReportedTrialSecond = -1;

	/** Seconds left of the pause before the next trial starts. */
	double SecondsUntilNextTrial = 0.0;

	/** True while a trial is running, false during the inter-trial pause. */
	bool bTrialActive = false;

	/** Radius around the pawn, in which item spawn points are drawn. */
	UPROPERTY(EditAnywhere, Category = "Example|Task")
	float ItemSpawnRadius = 2000.0f;

	/** Pause between the end of one trial and the start of the next, in seconds. */
	UPROPERTY(EditAnywhere, Category = "Example|Task")
	float SecondsBetweenTrials = 10.0f;
};
