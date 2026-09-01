// Copyright (c) Gallant Lab. All Rights Reserved.
//
// Portions of this file are adapted from the CARLA open-source autonomous driving simulator
// (https://github.com/carla-simulator/carla).
// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// Licensed under the MIT License (https://opensource.org/licenses/MIT).

#pragma once

#include "CoreMinimal.h"

class UMRIAgentComponent;
class UMRIPlayerAgentComponent;

/**
 * Semantic type of a tracked entity, written to the per-frame XML log.
 *
 * The plugin defines the player entry. Extend this enum with the entity types your
 * experiment tracks, add a UMRIAgentComponent subclass for each, and implement the
 * corresponding Visit() overload in your MRIExperimentState subclass.
 */
enum class EEntityType : uint8
{
	/** Entity whose agent component type was not recognized by any typed Visit() overload. */
	Unknown = 0,
	/** The subject's (player's) pawn. */
	Player,
};

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
	virtual ~IMRIAgentComponentVisitor() = default;

	/**
	 * Reads relevant state information from the given agent component.
	 *
	 * Called by UMRIAgentComponent::AcceptVisitor for agent component types that have
	 * no typed Visit overload. Subclasses of UMRIAgentComponent should override
	 * AcceptVisitor to call typed Visit overloads so we record appropriate info.
	 * @param agent  The agent component whose state should be recorded.
	 */
	virtual void Visit(const UMRIAgentComponent& agent) = 0;

	/**
	 * Reads state from the subject's player agent component.
	 * @param agent  The player agent component whose state should be recorded.
	 */
	virtual void Visit(const UMRIPlayerAgentComponent& agent) = 0;
};
