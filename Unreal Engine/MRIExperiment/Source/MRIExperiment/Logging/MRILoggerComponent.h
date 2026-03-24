// MRI Experiment Plugin - Base Logger Component

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "State/MRISubjectState.h"
#include "MRILoggerComponent.generated.h"

class AMRISpectatorController;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MRIEXPERIMENT_API UMRILoggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMRILoggerComponent();

	// Called every tick by the spectator controller to log frame information
	virtual void LogFrame();

	void SetParentController(AMRISpectatorController* parent);

protected:
	AMRISpectatorController* spectatorController = nullptr;
	AMRISubjectState* playerState = nullptr;
};
