// Copyright (c) Gallant Lab. All Rights Reserved.
//
// Portions of this file are adapted from the CARLA open-source autonomous driving simulator
// (https://github.com/carla-simulator/carla).
// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// Licensed under the MIT License (https://opensource.org/licenses/MIT).

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
	 * should override AcceptVisitor to call typed Visit overloads so we record appropriate info
	 * @param agent  The agent component whose state should be recorded.
	 */
	virtual void Visit(const UMRIAgentComponent &) = 0;
};
