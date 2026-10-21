// Copyright (c) Gallant Lab. All Rights Reserved.

#include "ExperimentSubjectState.h"

#include "Net/UnrealNetwork.h"

AExperimentSubjectState::AExperimentSubjectState()
{
	FrameNumber = 0u;
	SimulationStepInSeconds = 0.0f;
	PlatformTimeStamp = 0;
	GameTimeStamp = 0.0f;
	Transform = FTransform::Identity;
	TTL = false;
	TotalTTLs = 0;
	secondsToStartOfRun = 0.0f;
	Points = 0;
	displayedPromptType = EDisplayedPromptType::None;
}

void AExperimentSubjectState::Reset()
{
	Super::Reset();
	ResetExperimentState();
}

void AExperimentSubjectState::ResetExperimentState()
{
	Points = 0;
	displayedPromptType = EDisplayedPromptType::None;
	secondsToStartOfRun = 0.0f;
	TTL = false;
	TotalTTLs = 0;
}

void AExperimentSubjectState::CopyProperties(APlayerState *playerState)
{
	Super::CopyProperties(playerState);

	AExperimentSubjectState *subjectState = Cast<AExperimentSubjectState>(playerState);
	if (subjectState == nullptr)
	{
		return;
	}

	subjectState->Transform = Transform;
	subjectState->TTL = TTL;
	subjectState->TotalTTLs = TotalTTLs;
	subjectState->secondsToStartOfRun = secondsToStartOfRun;
	subjectState->Points = Points;
	subjectState->displayedPromptType = displayedPromptType;
}

void AExperimentSubjectState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	// Call DOREPLIFETIME for each variable that you eventually want to know about
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AExperimentSubjectState, Transform);
	DOREPLIFETIME(AExperimentSubjectState, TTL);
	DOREPLIFETIME(AExperimentSubjectState, TotalTTLs);
	DOREPLIFETIME(AExperimentSubjectState, secondsToStartOfRun);
	DOREPLIFETIME(AExperimentSubjectState, Points);
	DOREPLIFETIME(AExperimentSubjectState, displayedPromptType);
}

void AExperimentSubjectState::UpdateTimeStamp(float deltaSeconds)
{
	FrameNumber += 1u;
	SimulationStepInSeconds = deltaSeconds;
	PlatformTimeStamp = FPlatformTime::Seconds();
	GameTimeStamp += deltaSeconds;
}
