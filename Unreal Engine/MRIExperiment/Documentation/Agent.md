# Agent Module

**Files:** `Agent/MRIAgentComponent.h`, `Agent/MRIAgentComponent.cpp`, `Agent/MRIAgentComponentVisitor.h`

**Attribution:** The agent component and visitor pattern in this module were adapted from the [CARLA open-source autonomous driving simulator](https://github.com/carla-simulator/carla) (version 0.8.4). Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). Licensed under the [MIT License](https://opensource.org/licenses/MIT).

## What It Does

The agent module provides a visitor-pattern system for identifying and classifying entities in the world during experiment state logging. Any actor that should be tracked in the per-frame XML log needs one of the agent components attached to it.

`UMRIAgentComponent` is the abstract base class, a `USceneComponent` with an `AcceptVisitor()` method and a `GetId()` method that returns a hash-based unique identifier. For each concrete agent you implement, you also need to override `AcceptVisitor()` to call the correct `Visit()` overload on the visitor.

`IMRIAgentComponentVisitor` is a pure virtual interface with the `Visit()` method, one per concrete agent type. `MRIExperimentState` implements this interface: when it visits an agent, it creates the appropriate Entity State. This lets the experiment state record every tracked entity's position, rotation, type, and ID without the state class needing to know about specific actor classes.

## How to Use

For each actor class you want tracked during the experiment, create a matching `AgentComponent` that is attached to the class that tracks the relevant state information.
In the Visitor, overload the `Visit()` method to the appropriate component class that will read out the information saved by the component.
Add a new value to the `EntityType` enum, and implement the new `Visit()` in your `MRIExperimentState` subclass. 

During replay rendering, `MRIExperimentState::RecordStates()` iterates over all agent components and visits each one to build the entity state array. You do not need to call this manually; the logger component handles it.