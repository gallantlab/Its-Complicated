// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExperimentSubjectState.h"
#include "ExampleSubjectState.generated.h"

/**
 *  Player state for the example experiment.
 *  Inherits the replicated MRI experiment fields from AExperimentSubjectState.
 *  Add experiment-specific replicated fields here, overriding
 *  GetLifetimeReplicatedProps, CopyProperties, and ResetExperimentState
 *  (calling Super first in each), per the plugin's State documentation.
 */
UCLASS()
class AExampleSubjectState : public AExperimentSubjectState
{
	GENERATED_BODY()
};
