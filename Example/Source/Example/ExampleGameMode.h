// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ExampleGameMode.generated.h"

/**
 *  Game mode for the example experiment.
 *  Assigns the Experiment subject controller, subject player state, and replay
 *  spectator controller classes.
 */
UCLASS(abstract)
class AExampleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AExampleGameMode();
};



