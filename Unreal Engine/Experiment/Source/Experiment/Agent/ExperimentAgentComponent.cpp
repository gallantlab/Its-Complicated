// Experiment Plugin - Base Agent Component

#include "ExperimentAgentComponent.h"

UExperimentAgentComponent::UExperimentAgentComponent(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UExperimentAgentComponent::AcceptVisitor(IExperimentAgentComponentVisitor &Visitor) const
{
	Visitor.Visit(*this);
}

void UExperimentAgentComponent::BeginPlay()
{
	Super::BeginPlay();
	bAgentComponentIsRegistered = bRegisterAgentComponent;
}

void UExperimentAgentComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	bAgentComponentIsRegistered = false;
}
