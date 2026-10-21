// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExperimentLoggerComponent.h"
#include "ExampleLoggerComponent.generated.h"

/**
 *  Logger component for the example experiment.
 *  Each tick of replay rendering, records an ExampleExperimentState snapshot
 *  into the spectator controller's log buffer.
 */
UCLASS()
class UExampleLoggerComponent : public UExperimentLoggerComponent
{
	GENERATED_BODY()

public:

	/**
	 * Captures the current frame into an ExampleExperimentState and appends
	 * it to the spectator controller's log buffer.
	 */
	virtual void LogFrame() override;
};
