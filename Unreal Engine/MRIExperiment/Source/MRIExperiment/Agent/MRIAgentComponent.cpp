// MRI Experiment Plugin - Base Agent Component

#include "MRIAgentComponent.h"

UMRIAgentComponent::UMRIAgentComponent(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMRIAgentComponent::AcceptVisitor(IMRIAgentComponentVisitor &Visitor) const
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
