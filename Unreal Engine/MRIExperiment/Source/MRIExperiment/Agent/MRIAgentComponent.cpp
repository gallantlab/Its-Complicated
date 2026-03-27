// MRI Experiment Plugin - Base Agent Component

#include "MRIExperiment.h"
#include "Agent/MRIAgentComponent.h"

UMRIAgentComponent::UMRIAgentComponent(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMRIAgentComponent::AcceptVisitor(IMRIAgentComponentVisitor &Visitor)
{
	Visitor.Visit(*this);
}

void UMRIAgentComponent::BeginPlay()
{
	Super::BeginPlay();
	bAgentComponentIsRegistered = bRegisterAgentComponent;
}

void UMRIAgentComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	bAgentComponentIsRegistered = false;
}
