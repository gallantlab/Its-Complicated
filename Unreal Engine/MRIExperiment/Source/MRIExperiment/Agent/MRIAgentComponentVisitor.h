// MRI Experiment Plugin - Agent Component Visitor Interface

#pragma once

class UMRIAgentComponent;

class IMRIAgentComponentVisitor
{
public:
	// The visit method is for reading out the relevant state information
	virtual void Visit(const UMRIAgentComponent &) = 0;
};
