# Logging Module

**Files:** `Logging/MRIExperimentState.h`, `Logging/MRIExperimentState.cpp`, `Logging/MRILoggerComponent.h`, `Logging/MRILoggerComponent.cpp`

## What It Does

This module has two classes that work together to produce per-frame snapshots of the experiment during replay rendering.

`MRIExperimentState` represents a single frame's worth of data. It stores the frame time and number, the subject's controls (speed, throttle, steering, brake, gear, handbrake), MRI state (TTL, total TTLs, beep, points, displayed prompt type), and an array of `EntityState` structs capturing the position, rotation, type, and ID of every tracked entity in the world. It populates the entity array using the visitor pattern: it iterates over all `UMRIAgentComponent` instances, and each component's `AcceptVisitor()` dispatches to the appropriate `Visit()` overload which creates an `EntityState` with the correct `EntityType` . The `WriteToFile()` method serializes a `<Frame>` XML tag containing TTL attributes, player control attributes via the virtual `WriteContents()`, and entity position/rotation tags.

`UMRILoggerComponent` is an actor component that lives on the spectator controller. Each tick during replay rendering, the spectator calls `LogFrame()`, which creates a new `MRIExperimentState` from the current demo time, frame number, world agents, and reference player state, then adds it to the spectator's frame state array.

## How to Use

You do not interact with these classes during live gameplay. During replay rendering, the spectator controller automatically creates a logger component (via `SpawnLoggerComponent()`) and calls `LogFrame()` each tick. The accumulated states are written to XML by `SaveExperimentLog()`.

You need to subclass these two classes to implement logging whatever information you want out of your experiment:

Inherit from `MRIExperimentState` and override `WriteContents()`. Call Super to write the standard player controls and entity positions, then append your custom XML tags for whatever you want to track.

Inherit from `UMRILoggerComponent` and override `LogFrame()`. Create an instance of your custom experiment state subclass instead of the base `MRIExperimentState`, populate any experiment-specific fields from the reference player state (which you should cast to your experiment's player state subclass), and add it to the spectator controller via `AddExperimentState()`.

Then override `SpawnLoggerComponent()` on your spectator controller subclass to instantiate your custom logger instead of the base one.
