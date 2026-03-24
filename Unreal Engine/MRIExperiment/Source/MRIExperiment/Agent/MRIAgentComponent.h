// MRI Experiment Plugin - Base Agent Component

#pragma once

#include "Components/SceneComponent.h"
#include "Agent/MRIAgentComponentVisitor.h"
#include "MRIAgentComponent.generated.h"

UCLASS(Abstract)
class MRIEXPERIMENT_API UMRIAgentComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	UMRIAgentComponent(const FObjectInitializer &ObjectInitializer);

	uint32 GetId() const
	{
		return GetTypeHash(this);
	}

	virtual void AcceptVisitor(IMRIAgentComponentVisitor &Visitor);

protected:

	void BeginPlay() override;

	void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere)
	bool bRegisterAgentComponent = true;

	UPROPERTY(VisibleAnywhere, AdvancedDisplay)
	bool bAgentComponentIsRegistered = false;
};
