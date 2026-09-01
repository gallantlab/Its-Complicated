// MRI Experiment Plugin - Player Agent Component

#include "Agent/MRIPlayerAgentComponent.h"

UMRIPlayerAgentComponent::UMRIPlayerAgentComponent(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMRIPlayerAgentComponent::AcceptVisitor(IMRIAgentComponentVisitor &Visitor) const
{
	Visitor.Visit(*this);
}
