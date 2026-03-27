// MRI Experiment Plugin - Base Agent Component

#include "MRIExperiment.h"
#include "Agent/MRIAgentComponent.h"

/** Constructs the component with ticking disabled; agents are polled on demand by the logging system. */
UMRIAgentComponent::UMRIAgentComponent(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

/** Calls Visitor.Visit(*this) to let the visitor read this component's state. */
void UMRIAgentComponent::AcceptVisitor(IMRIAgentComponentVisitor &Visitor)
{
	Visitor.Visit(*this);
}

/** Sets bAgentComponentIsRegistered based on bRegisterAgentComponent. */
void UMRIAgentComponent::BeginPlay()
{
	Super::BeginPlay();
	bAgentComponentIsRegistered = bRegisterAgentComponent;
}

/** Clears bAgentComponentIsRegistered when the actor leaves the world. */
void UMRIAgentComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	bAgentComponentIsRegistered = false;
}
