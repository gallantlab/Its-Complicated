// Copyright (c) Gallant Lab. All Rights Reserved.

#include "MRISubjectState.h"

#include "Net/UnrealNetwork.h"

AMRISubjectState::AMRISubjectState()
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

void AMRISubjectState::Reset()
{
	Super::Reset();
	ResetExperimentState();
}

void AMRISubjectState::ResetExperimentState()
{
	Points = 0;
	displayedPromptType = EDisplayedPromptType::None;
	secondsToStartOfRun = 0.0f;
	TTL = false;
	TotalTTLs = 0;
}

void AMRISubjectState::CopyProperties(APlayerState *playerState)
{
	Super::CopyProperties(playerState);

	AMRISubjectState *subjectState = Cast<AMRISubjectState>(playerState);
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

void AMRISubjectState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	// Call DOREPLIFETIME for each variable that you eventually want to know about
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMRISubjectState, Transform);
	DOREPLIFETIME(AMRISubjectState, TTL);
	DOREPLIFETIME(AMRISubjectState, TotalTTLs);
	DOREPLIFETIME(AMRISubjectState, secondsToStartOfRun);
	DOREPLIFETIME(AMRISubjectState, Points);
	DOREPLIFETIME(AMRISubjectState, displayedPromptType);
}

void AMRISubjectState::UpdateTimeStamp(float deltaSeconds)
{
	FrameNumber += 1u;
	SimulationStepInSeconds = deltaSeconds;
	PlatformTimeStamp = FPlatformTime::Seconds();
	GameTimeStamp += deltaSeconds;
}
