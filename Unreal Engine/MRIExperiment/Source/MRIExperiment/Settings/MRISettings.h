// Copyright (c) Gallant Lab. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MRISettings.generated.h"

UENUM(BlueprintType)
// Add experiment type specifiers here
enum class EExperimentType : uint8
{
	Other              UMETA(DisplayName = "Other")
};

UCLASS(BlueprintType)
class MRIEXPERIMENT_API UExperimentType : public UObject
{
	GENERATED_BODY()

public:

	using uint_type = typename std::underlying_type<EExperimentType>::type;

	static EExperimentType FromString(const FString &String);

	static FString ToString(EExperimentType ExperimentType);

	static constexpr uint_type ToUInt(EExperimentType ExperimentType)
	{
		return static_cast<uint_type>(ExperimentType);
	}
};

UCLASS(BlueprintType, Blueprintable, config = Game, defaultconfig)
class MRIEXPERIMENT_API UMRISettings : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "MRI")
	void SetExperimentType(EExperimentType InExperimentType);

	UFUNCTION(BlueprintCallable, Category = "MRI")
	EExperimentType GetExperimentType() const;

	/** Load settings from the default config file and command line overrides. */
	void LoadSettings();

	UFUNCTION(BlueprintCallable, Category = "MRI")
	void LoadSettingsFromFile(const FString &FilePath, bool bLogOnFailure = true);

	void LoadSettingsFromString(const FString &INIStyleString);

	UFUNCTION(BlueprintCallable, Category = "MRI")
	bool SaveSettings();

	UFUNCTION(BlueprintCallable, Category = "MRI")
	bool SaveSettingsToFile(const FString &FilePath);

	void LogSettings() const;

	UFUNCTION(BlueprintCallable, Category = "MRI")
	void SetResolution(int Width, int Height, bool Fullscreen = false);

	UFUNCTION(BlueprintPure, Category = "MRI")
	const FString &GetFileName() const
	{
		return CurrentFileName;
	}

public:

	UPROPERTY(Category = "MRI", EditAnywhere, BlueprintReadWrite)
	FString Subject;

	UPROPERTY(Category = "MRI", EditAnywhere, BlueprintReadWrite)
	bool AutoTriggerDemoRecording = true;

	UPROPERTY(Category = "MRI", EditAnywhere, BlueprintReadWrite)
	bool AutoEyetrackingCalibration = false;

	UPROPERTY(Category = "MRI", EditAnywhere, BlueprintReadWrite)
	float SecondsToDemoStop = 0.0f;

	UPROPERTY(Category = "MRI", VisibleAnywhere, BlueprintReadOnly)
	bool SensorsOnlyOnReplay = true;

	UPROPERTY(Category = "MRI", EditAnywhere, BlueprintReadWrite)
	FString Resolution = TEXT("1280x800w");

	UPROPERTY(Category = "MRI", VisibleAnywhere, BlueprintReadOnly)
	int RenderAll = 0;

	UPROPERTY(Category = "MRI", EditAnywhere, BlueprintReadWrite)
	int TTLsBeforeExperimentStart = 5;

	UPROPERTY(Category = "Sensors", VisibleAnywhere, BlueprintReadOnly)
	bool bSemanticSegmentationEnabled = false;

private:

	UPROPERTY(Category = "MRI", VisibleAnywhere)
	EExperimentType ExperimentType = EExperimentType::Other;

	FString CurrentFileName;
};
