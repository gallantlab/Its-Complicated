// Copyright (c) Gallant Lab. All Rights Reserved.
//
// Portions of this file are adapted from the CARLA open-source autonomous driving simulator
// (https://github.com/carla-simulator/carla).
// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// Licensed under the MIT License (https://opensource.org/licenses/MIT).

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MRISettings.generated.h"

/**
 * Identifies the type of experiment being run.
 *
 * Add new experiment type specifiers here and update UExperimentType::ToString/FromString.
 */
UENUM(BlueprintType)
// Add experiment type specifiers here
enum class EExperimentType : uint8
{
	/** Generic or unspecified experiment type. */
	Other              UMETA(DisplayName = "Other")
};

/**
 * Utility class providing string conversion and numeric casting for EExperimentType.
 */
UCLASS(BlueprintType)
class MRIEXPERIMENT_API UExperimentType : public UObject
{
	GENERATED_BODY()

public:

	/** Underlying integer type of the EExperimentType enum. */
	using uint_type = typename std::underlying_type<EExperimentType>::type;

	/**
	 * Parses an EExperimentType value from its display name.
	 * @param String  The string representation of the experiment type.
	 * @return The matching EExperimentType value, or EExperimentType::Other if unrecognized.
	 */
	static EExperimentType FromString(const FString &String);

	/**
	 * Returns the display name for the given experiment type.
	 * @param ExperimentType  The enum value to convert.
	 * @return The display name for that experiment type.
	 */
	static FString ToString(EExperimentType ExperimentType);

	/**
	 * Casts an EExperimentType value to its underlying integer representation.
	 * @param ExperimentType  The enum value to cast.
	 * @return The integer value of the enum entry.
	 */
	static constexpr uint_type ToUInt(EExperimentType ExperimentType)
	{
		return static_cast<uint_type>(ExperimentType);
	}
};

/**
 * Per-session configuration loaded from an INI-style settings file.
 *
 * Settings are loaded at startup from the default game config file and can be
 * overridden via command-line arguments or by calling LoadSettingsFromFile.
 * Exposes all configurable parameters to Blueprints for runtime inspection.
 */
UCLASS(BlueprintType, Blueprintable, config = Game, defaultconfig)
class MRIEXPERIMENT_API UMRISettings : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Sets the experiment type for this session.
	 * @param InExperimentType  The experiment type to apply.
	 */
	UFUNCTION(BlueprintCallable, Category = "MRI")
	void SetExperimentType(EExperimentType InExperimentType);

	/**
	 * Returns the experiment type for this session.
	 * @return The currently configured EExperimentType.
	 */
	UFUNCTION(BlueprintCallable, Category = "MRI")
	EExperimentType GetExperimentType() const;

	/**
	 * Loads settings from the default config file and applies any command-line overrides.
	 */
	void LoadSettings();

	/**
	 * Loads settings from the specified INI-format file.
	 * @param FilePath       Absolute path to the settings file.
	 * @param bLogOnFailure  If true, logs an error if the file cannot be read.
	 */
	UFUNCTION(BlueprintCallable, Category = "MRI")
	void LoadSettingsFromFile(const FString &FilePath, bool bLogOnFailure = true);

	/**
	 * Loads settings from an INI-format string in memory.
	 * @param INIStyleString  A string containing key=value pairs in INI format.
	 */
	void LoadSettingsFromString(const FString &INIStyleString);

	/**
	 * Saves the current settings back to the file they were loaded from.
	 * @return true if the settings were saved successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "MRI")
	bool SaveSettings();

	/**
	 * Saves the current settings to the specified file path.
	 * @param FilePath  Absolute path of the output settings file.
	 * @return true if the file was written successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "MRI")
	bool SaveSettingsToFile(const FString &FilePath);

	/**
	 * Logs all current setting values via LogMRI at the Log verbosity level.
	 */
	void LogSettings() const;

	/**
	 * Try to set the current display resolution.
	 * @param Width       Desired display width in pixels.
	 * @param Height      Desired display height in pixels.
	 * @param Fullscreen  If true, switches to fullscreen mode.
	 */
	UFUNCTION(BlueprintCallable, Category = "MRI")
	void SetResolution(int Width, int Height, bool Fullscreen = false);

	/**
	 * Returns the path of the file that was most recently loaded.
	 * @return Absolute path of the current settings file.
	 */
	UFUNCTION(BlueprintPure, Category = "MRI")
	const FString &GetFileName() const
	{
		return CurrentFileName;
	}

public:

	/** Subject identifier / participant code. */
	UPROPERTY(Category = "MRI", EditAnywhere, BlueprintReadWrite)
	FString Subject;

	/** If true, demo recording begins automatically on the first TTL. */
	UPROPERTY(Category = "MRI", EditAnywhere, BlueprintReadWrite)
	bool AutoTriggerDemoRecording = true;

	/** If true, eyetracking calibration is triggered automatically at the start of each run. */
	UPROPERTY(Category = "MRI", EditAnywhere, BlueprintReadWrite)
	bool AutoEyetrackingCalibration = false;

	/** Seconds after the last TTL before demo recording stops automatically. 0 disables auto-stop. */
	UPROPERTY(Category = "MRI", EditAnywhere, BlueprintReadWrite)
	float SecondsToDemoStop = 0.0f;

	/** If true, rendering is only enabled during replay, not during the experiment. */
	UPROPERTY(Category = "MRI", VisibleAnywhere, BlueprintReadOnly)
	bool SensorsOnlyOnReplay = true;

	/** Display resolution string in Unreal format (<Width>x<Height>[w|f]). */
	UPROPERTY(Category = "MRI", EditAnywhere, BlueprintReadWrite)
	FString Resolution = TEXT("1280x800w");

	/** Autorendering flag; 0 is disabled. Any value is the framerate to render at */
	UPROPERTY(Category = "MRI", VisibleAnywhere, BlueprintReadOnly)
	int RenderAll = 0;

	/** Number of TTLs to wait for before experiment logic begins each run. */
	UPROPERTY(Category = "MRI", EditAnywhere, BlueprintReadWrite)
	int TTLsBeforeExperimentStart = 5;

	/** DO we do semseg on this current lifecycle?. */
	UPROPERTY(Category = "Sensors", VisibleAnywhere, BlueprintReadOnly)
	bool bSemanticSegmentationEnabled = false;

private:

	/** Experiment type for this session; set via SetExperimentType or loaded from config. */
	UPROPERTY(Category = "MRI", VisibleAnywhere)
	EExperimentType ExperimentType = EExperimentType::Other;

	/** Absolute path of the most recently loaded settings file. */
	FString CurrentFileName;
};
