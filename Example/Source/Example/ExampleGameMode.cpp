// Copyright Epic Games, Inc. All Rights Reserved.

#include "ExampleGameMode.h"

#include "ExamplePlayerController.h"
#include "ExampleSpectatorController.h"
#include "ExampleSubjectState.h"

AExampleGameMode::AExampleGameMode()
{
	PlayerControllerClass = AExamplePlayerController::StaticClass();
	PlayerStateClass = AExampleSubjectState::StaticClass();
	ReplaySpectatorPlayerControllerClass = AExampleSpectatorController::StaticClass();
}
