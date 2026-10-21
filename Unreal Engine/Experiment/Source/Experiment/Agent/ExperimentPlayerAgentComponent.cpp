// Experiment Plugin - Player Agent Component

#include "Agent/ExperimentPlayerAgentComponent.h"

UExperimentPlayerAgentComponent::UExperimentPlayerAgentComponent(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UExperimentPlayerAgentComponent::AcceptVisitor(IExperimentAgentComponentVisitor &Visitor) const
{
	Visitor.Visit(*this);
}
