# Spectator Controller Module

**Files:** `Demo/ExperimentSpectatorController.h`, `Demo/ExperimentSpectatorController.cpp`

## What It Does

`AExperimentSpectatorController` is the player controller that takes over during demo replay. Its job is to step through the replay frame-by-frame at a deterministic timestep, save camera images to disk, log per-frame experiment state, and write the final XML log when the replay ends.

When the controller is created during a replay (`PostActorCreated`), it checks the demo subsystem for whether frame capture has been requested. If so, it sets a fixed timestep based on the desired FPS, pauses the world, grabs a reference to the `DemoNetDriver`, and enables frame capture mode. It then calls `TrySetViewTargetToPlayer()` to find the subject's pawn (by tag or cached name), attach to it, and cast its player state to `AExperimentSubjectState`.

The `Tick` function has three phases. First, a delay phase (default 5 seconds) lets the replay world fully load while paused. Second, the render phase unpauses the world, calls `CaptureCameras()` and `LogExperimentState()` each tick, and checks whether the demo time has stopped advancing (indicating the replay has ended). Third, once the replay ends, it writes the XML log, stops the demo net driver, and restarts the level.

`SaveExperimentLog()` writes an XML file named `{demoName}-log.xml` containing the experiment type, player ID, capture FPS, seconds to run start, static entities (road signs from frame 0), and all per-frame states. Each frame's state is written by `ExperimentState::WriteToFile()`.

`FindTaggedPlayerPawn()` iterates all `APawn` actors looking for the `ExperimentPlayerTag` tag.

## How to Use

Set `AExperimentSpectatorController` (or your subclass) as the spectator class in your game mode. The demo subsystem will spawn it automatically during replay. You should not need to interact with it directly during live gameplay.

Things that you need to implement
- `AttachCamera()` -- override to attach each settings-file camera to a specific component, such as the pawn's gameplay camera; the base implementation attaches to the subject pawn's root. `CreateCameras()` spawns one `AExperimentFrameCaptureCamera` per settings-file entry once the subject pawn is known; each camera receives its own save subfolder and a `SaveNextFrame()` call every render tick.
- `SpawnLoggerComponent()` -- the most important override. The base implementation creates a generic `UExperimentLoggerComponent`. Override this to instantiate your experiment-specific logger subclass, which will create experiment-specific `ExperimentState` subclasses with additional per-frame data.
- `SaveExperimentLog()` -- override to add custom XML sections (like destination lists, trial parameters, or other experiment metadata) to the output file. Call Super to get the standard output, or rewrite entirely.
