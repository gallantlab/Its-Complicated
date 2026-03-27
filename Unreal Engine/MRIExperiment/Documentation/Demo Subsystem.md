# Demo Subsystem Module

**Files:** `Demo/MRIDemoSubsystem.h`, `Demo/MRIDemoSubsystem.cpp`

**Reference:** The demo recording and playback logic was implemented with reference to the [Unreal Engine community replay system tutorial](https://unrealcommunity.wiki/replay-system-tutorial-41kq5b58). 

## What It Does

`UMRIDemoSubsystem` is a `UGameInstanceSubsystem` that manages demo recording, playback, and replay rendering. Because it is a subsystem, it gets automatically created alongside the game instance and persists across level loads, but does not require you to replace the default game instance class.

On initialization, it creates a `UMRISettings` object, loads the config, sets up the replay streaming infrastructure for enumerating saved demos, and prepares delegates for replay enumeration and deletion callbacks. It tracks the current demo state as an integer (0 = no demo, 1 = recording, 2 = ended) which the controller polls each tick.

For recording, `StartRecordingReplay()` delegates to the engine's built-in `UGameInstance::StartRecordingReplay()`, generates a timestamp-based filename if none is provided, and sends a WebSocket message to OBS to start video recording. `StopRecordingReplay()` does the reverse.

For rendering, `DemoRenderFrames(demoName, fps)` starts replay of a demo and sets flags that the spectator controller picks up to begin frame capture. It immediately pauses the replay so the spectator can control the timestep.

The subsystem also handles player pawn identification in the replays, and replay enumeration with rendered-status tracking (checking for the existence of a `-log.xml` file).

## How to Use

Access the subsystem from any actor via `UGameInstance::GetSubsystem<UMRIDemoSubsystem>(GetGameInstance())`. The controller does this automatically in `BeginPlay`.

To manually trigger recording from Blueprint, use `StartRecordingReplayFromBP` and `StopRecordingReplayFromBP`. To trigger replay rendering, call `DemoRenderFrames` with the demo name and desired FPS.

To enumerate available replays (for building a replay menu UI), call `FindReplays()` and bind to `BP_OnFindReplaysComplete` in Blueprint, or read `GetDemosList()` and `GetIsDemoRenderedList()` for the current list.

Settings are accessible via `GetSettings()` which returns the `UMRISettings` reference, or `GetMRISettings()` for Blueprint access.

## Extension Points

The subsystem itself is not typically subclassed. Instead, it serves as infrastructure that the controller, spectator, and UI interact with.

`RequestOBSRecording()` is a stub that logs a warning. If your lab uses OBS for synchronized video recording, provide a WebSocket client implementation here.
