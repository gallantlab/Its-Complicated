// THis is the core class that records state. Everything you want to know needs to be stored in here.

#pragma once

#include "GameFramework/PlayerState.h"
#include "MRISubjectState.generated.h"

UENUM(BlueprintType)
enum class EDisplayedPromptType : uint8
{
	None,
	Unknown,
	SessionEnd,
	Paused,
};

UCLASS()
class MRIEXPERIMENT_API AMRISubjectState : public APlayerState
{
	GENERATED_BODY()

public:

	AMRISubjectState();

	virtual void Reset() override;

	virtual void ResetExperimentState();

	virtual void CopyProperties(APlayerState *playerState) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &outLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	uint64 GetFrameNumber() const
	{
		return FrameNumber;
	}

	UFUNCTION(BlueprintCallable)
	float GetSimulationStepInSeconds() const
	{
		return SimulationStepInSeconds;
	}

	UFUNCTION(BlueprintCallable)
	int32 GetPlatformTimeStamp() const
	{
		return PlatformTimeStamp;
	}

	UFUNCTION(BlueprintCallable)
	int32 GetGameTimeStamp() const
	{
		return GameTimeStamp;
	}

	UFUNCTION(BlueprintCallable)
	bool isTTL() const
	{
		return TTL;
	}

	UFUNCTION(BlueprintCallable)
	bool IsBeep() const
	{
		return PlayBeep;
	}

	UFUNCTION(BlueprintCallable)
	const FTransform &GetTransform() const
	{
		return Transform;
	}

	UFUNCTION(BlueprintCallable)
	FVector GetLocation() const
	{
		return Transform.GetLocation();
	}

	UFUNCTION(BlueprintCallable)
	FVector GetOrientation() const
	{
		return Transform.GetRotation().GetForwardVector();
	}

	UFUNCTION(BlueprintCallable)
	float GetSecondsToStartOfRun() const
	{
		return secondsToStartOfRun;
	}

	UFUNCTION(BlueprintCallable)
	int GetPoints() const
	{
		return Points;
	}

	UFUNCTION(BlueprintCallable)
	EDisplayedPromptType GetDisplayedPromptType() const
	{
		return displayedPromptType;
	}

private:

	friend class AMRISubjectController;

	UPROPERTY(VisibleAnywhere)
	uint64 FrameNumber;

	UPROPERTY(VisibleAnywhere)
	float SimulationStepInSeconds;

	UPROPERTY(VisibleAnywhere)
	int32 PlatformTimeStamp;

	UPROPERTY(VisibleAnywhere)
	int32 GameTimeStamp = 0.0f;

	UPROPERTY(Replicated)
	FTransform Transform;

	UPROPERTY(Replicated)
	bool TTL = false;

	UPROPERTY(Replicated)
	int TotalTTLs = 0;

	UPROPERTY(Replicated)
	bool PlayBeep = false;

	UPROPERTY(Replicated)
	float secondsToStartOfRun = 0.0;

	UPROPERTY(Replicated)
	int Points = 0;

	UPROPERTY(Replicated)
	EDisplayedPromptType displayedPromptType = EDisplayedPromptType::None;

	void UpdateTimeStamp(float deltaSeconds);
};
