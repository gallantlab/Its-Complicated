# Logging Module

**Files:** `Logging/MRIExperimentState.h`, `Logging/MRIExperimentState.cpp`, `Logging/MRILoggerComponent.h`, `Logging/MRILoggerComponent.cpp`

## What It Does

This module has two classes that work together to produce per-frame snapshots of the experiment during replay rendering.

`MRIExperimentState` represents a single frame's worth of data. It stores the frame time and number, MRI state (TTL, total TTLs, points, displayed prompt type), and an array of `EntityState` structs capturing the position, rotation, type, and ID of every tracked entity in the world. It populates the entity array using the visitor pattern: it iterates over all `UMRIAgentComponent` instances, and each component's `AcceptVisitor()` dispatches to the appropriate `Visit()` overload which creates an `EntityState` with the correct `EntityType` . The `WriteToFile()` method serializes a `<Frame>` XML tag with frame number, time, TTL, and points attributes, then writes a `<Player>` prompt tag and entity position/rotation tags via the virtual `WriteContents()`.

`UMRILoggerComponent` is an actor component that lives on the spectator controller. Each tick during replay rendering, the spectator calls `LogFrame()`, which creates a new `MRIExperimentState` from the current demo time, frame number, world agents, and reference player state, then adds it to the spectator's frame state array.

## How to Use

You do not interact with these classes during live gameplay. During replay rendering, the spectator controller automatically creates a logger component (via `SpawnLoggerComponent()`) and calls `LogFrame()` each tick. The accumulated states are written to XML by `SaveExperimentLog()`.

You need to subclass these two classes to implement logging whatever information you want out of your experiment:

Inherit from `MRIExperimentState`. Define a constructor that takes the frame data and calls `Record()` in its body — by that point the object is fully constructed as your subclass, so your `Visit()` overrides are the ones that run (a virtual call from inside a base constructor would run the base version, which is why the base class does not record during construction):

```cpp
ExampleExperimentState(double time, int frame, TArray<const UMRIAgentComponent*> agents, AMRISubjectState* playerState)
	: MRIExperimentState(time, frame)
{
	Record(agents, playerState);
}
```

Override `Visit()` for each agent component type you use, setting `thisType` to the matching `EEntityType` value; the base implementations crash deliberately so an unhandled type cannot go unnoticed. Override `WriteContents()` (call Super first to write the prompt and entity positions) to append your custom XML tags.

Inherit from `UMRILoggerComponent` and override `LogFrame()`. Create an instance of your experiment state subclass from the spectator controller's current demo time, frame number, agents, and reference player state (cast to your experiment's player state subclass), and add it to the spectator controller via `AddExperimentState()`.

Then override `SpawnLoggerComponent()` on your spectator controller subclass to instantiate your custom logger instead of the base one.
