// Copyright (c) Gallant Lab. All Rights Reserved.
// Spectator controller for replaying and rendering MRI experiment demos.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Sensor/MRIFrameCaptureCamera.h"
#include "Logging/MRIExperimentState.h"
#include "Logging/MRILoggerComponent.h"

#include "MRISpectatorController.generated.h"

class UMRIDemoSubsystem;
class AMRISubjectState;

UCLASS()
class MRIEXPERIMENT_API AMRISpectatorController : public APlayerController
{
	GENERATED_BODY()

public:

	AMRISpectatorController(const FObjectInitializer& objectInitializer);

	~AMRISpectatorController();

	virtual void Tick(float deltaTime) override;

	void SetReferencePawn(APawn* referencePawn);

	virtual void PostActorCreated() override;

	virtual void BeginPlayingState() override;

	APawn* FindTaggedPlayerPawn();

	void CaptureSensors();

	bool captureFrames;

	void SetFrameCaptureFolder(const FString& folder);

	virtual void SaveExperimentLog();

	bool IsStale() { return stopDemo; }

	void AddExperimentState(MRIExperimentState* newState);

	AMRISubjectState* GetReferencePlayerState() const
	{
		return referencePlayerState;
	}

	UDemoNetDriver* GetDemoNetDriver() const
	{
		return demoNetDriver;
	}

	UMRIDemoSubsystem* GetDemoSubsystem() const
	{
		return demoSubsystem;
	}

	int GetFrameNumber() const
	{
		return frameNumber;
	}

protected:

	void TrySetViewTargetToPlayer();

	void FixTimeStep(bool fix);

	void LogExperimentState();

	virtual void SpawnLoggerComponent();

private:

	APawn* referencePawn = nullptr;

	UMRILoggerComponent* loggerComponent = nullptr;

	AMRISubjectState* referencePlayerState = nullptr;

	TArray<AMRIFrameCaptureCamera*>* sensors = nullptr;

	bool needReference = true;

	int fps = 0;

	double currentTime = -1.0;

	double timeStep = 0.0;

	double delay = 5.0;

	UDemoNetDriver* demoNetDriver = nullptr;

	AWorldSettings* worldSettings = nullptr;

	TArray<MRIExperimentState*>* frameStates = nullptr;

	EExperimentType experimentType = EExperimentType::Other;

	int frameNumber;

	uint32 playerAgentID;

	UMRIDemoSubsystem* demoSubsystem = nullptr;

	bool stopDemo = false;
};
