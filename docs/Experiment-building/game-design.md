# Game Design Concepts for MRI Experiments

## Overview

[Placeholder: motivation for using game engines in neuroscience experiments. Why Unreal Engine and not a traditional stimulus delivery system like PsychoPy or E-Prime.]

## Why a Game Engine

[Placeholder: real-time 3D rendering, physics simulation, network replication, built-in demo recording, and a rich asset pipeline make game engines suitable for complex closed-loop experiments that traditional stimulus tools were not designed for.]

## Core Game Design Concepts Used in This Framework

### Game Instance

[Placeholder: the `UGameInstance` lives for the entire lifetime of the running game, spanning level transitions. It is the right place to own singletons that must persist — such as the Eyelink connection — because their C-library initialization cost makes repeated init/teardown impractical.]

### Player Controller

[Placeholder: the `APlayerController` represents the human input side. In MRI experiments it also owns the TTL pulse handling and experiment lifecycle because those are fundamentally input/control-flow concerns. The distinction between the controller (what the subject does) and the pawn (what moves in the world) is important for replay fidelity.]

### Player State

[Placeholder: `APlayerState` is replicated automatically by Unreal's network layer and therefore gets baked into demo files. Storing all per-frame data the analysis pipeline needs in a replicated PlayerState is what makes post-hoc log generation from replays possible without re-running the experiment.]

### Game Mode

[Placeholder: `AGameModeBase` is server-authoritative and is the right place for commands that should affect the whole session — such as triggering eyetracking calibration — regardless of which controller or pawn is active.]

### Demo Recording and Replay

[Placeholder: Unreal's built-in demo recording system (`DemoNetDriver`) records network traffic as a `.demo` file. On replay the driver drives all replicated actors from the recorded data. This gives us a deterministic replay of every session at arbitrary frame rates, decoupled from real time, which is exactly what we need for frame-accurate stimulus rendering.]

### Subsystem

[Placeholder: `UGameInstanceSubsystem` allows functionality to be added to any game instance without replacing it. This is how `UMRIDemoSubsystem` is structured — it attaches to whatever `UGameInstance` the project already uses, so the plugin does not force a project-wide base class change.]

### Actor Components and the Scene Hierarchy

[Placeholder: Agent components, sensor cameras, logger components. How composition over inheritance lets you mix and match experiment features.]

## Experiment Lifecycle

[Placeholder: describe the sequence from game launch through session end. Scanner ready → TTL arrives → auto-trigger recording → `ExperimentTick` begins → session ends → demo recorded → replay → frame capture → log written.]

## Adapting These Concepts to Other Engines

[Placeholder: note from the Readme that the logic can be lifted to Unity or other engines. Identify which concepts are engine-agnostic and which depend on Unreal specifics.]
