// Copyright (c) Gallant Lab. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExperimentSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "ExperimentSettingsSubsystem.generated.h"

/**
 * Game instance subsystem that owns the experiment settings for a session.
 *
 * Created automatically alongside the game instance. On Initialize it creates the
 * UExperimentSettings object and loads it from Config/ExperimentSettings.ini (and the -experiment-settings=
 * command-line override). Consumers fetch settings through GetSettings(), or
 * GetExperimentSettings() from Blueprint.
 */
UCLASS()
class EXPERIMENT_API UExperimentSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/**
	 * Called by Unreal when the subsystem is created. Creates the UExperimentSettings
	 * object, loads it from file, and logs the loaded values.
	 * @param Collection  The subsystem collection managing this subsystem.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Called by Unreal when the subsystem is destroyed at session end.
	 * Calls SaveSubjectStateFile.
	 */
	virtual void Deinitialize() override;

	/**
	 * Returns a reference to the current Experiment settings object.
	 * @return Reference to the loaded UExperimentSettings instance.
	 */
	UExperimentSettings& GetSettings();

	/**
	 * Returns a pointer to the current Experiment settings for Blueprint access.
	 * @return Pointer to the UExperimentSettings object.
	 */
	UFUNCTION(BlueprintPure, Category = "Experiment")
	UExperimentSettings* GetExperimentSettings()
	{
		return settings;
	}

	/**
	 * Returns whether the display resolution has already been applied this session.
	 * @return true if the resolution has been set.
	 */
	bool HasResolutionBeenSet() const
	{
		return bIsResolutionSet;
	}

	/**
	 * Records that the display resolution has been applied for this session.
	 */
	void UpdateResolutionHasBeenSet()
	{
		bIsResolutionSet = true;
	}

	/**
	 * Called during Deinitialize at the end of a session.
	 * The base plugin has no knowledge of the shape of experiment state, so this
	 * implementation does nothing. Override in your project to persist subject
	 * state between sessions.
	 */
	virtual void SaveSubjectStateFile() {}

private:

	/** Loaded Experiment settings for this session. */
	UPROPERTY()
	TObjectPtr<UExperimentSettings> settings;

	/** True if the display resolution has already been applied this session. */
	bool bIsResolutionSet = false;
};
