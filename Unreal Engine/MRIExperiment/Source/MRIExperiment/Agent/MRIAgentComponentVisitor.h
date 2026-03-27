// MRI Experiment Plugin - Agent Component Visitor Interface

#pragma once

class UMRIAgentComponent;

/**
 * Visitor interface for reading state from MRI agent components.
 *
 * Implement this interface and pass an instance to UMRIAgentComponent::AcceptVisitor
 * to retrieve the current state of each agent component. The visitor pattern
 * decouples the logging/state-recording logic from the component hierarchy.
 */
class IMRIAgentComponentVisitor
{
public:
	/**
	 * Reads relevant state information from the given agent component.
	 *
	 * Called by UMRIAgentComponent::AcceptVisitor. Subclasses of UMRIAgentComponent
	 * may override AcceptVisitor to call typed Visit overloads, enabling visitors to
	 * distinguish between different agent types.
	 * @param agent  The agent component whose state should be recorded.
	 */
	virtual void Visit(const UMRIAgentComponent &) = 0;
};
