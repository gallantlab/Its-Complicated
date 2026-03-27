# MRIExperiment Plugin Overview

<img width="1849" height="1040" alt="image" src="https://github.com/user-attachments/assets/69882786-2112-46c3-85c5-3bb92543e297" />

The MRIExperiment plugin provides a framework for Unreal Engine-based closed loop MRI experiments. It provides logic for recordding TTLs, replicating experiment info, extracting info from recorded demos, and misc config stuff. The is more of a reference than a specific framework to use, but theoretically, you should be able to subclass from these classes and make runnable experiments..

The PlayerController handles UX and also TTLs, since that comes in on a HID object to the computer. Experiemnt-relevant info is replicated. After the session, the Demo Subsystem plays back each recorded demo and spawns a Spectator Controller, which steps through the replay frame-by-frame at a fixed timestep, and writes out info in a human-readable/data-analysis-parsable format. There's also a Setting module for handling runtime experiment configs.

## Modules

**Controller** (`AMRISubjectController`): The player controller that the subject uses during an experiment. It binds to TTL input events from the scanner, counts TTLs until the experiment should start, manages auto-triggering and auto-stopping demo recording, runs experiment-specific tick logic, and provides a delegate-based system for displaying text prompts to the subject. Inherit from this class and override `ExperimentTick()` to implement your experiment.

**State** (`AMRISubjectState`): The replicated player state that travels with the subject's pawn. It stores the subject's transform, TTL state, points, and the currently displayed prompt type. All properties are replicated so they get recorded into demo files and are available during replay. Inherit from this to add experiment-specific replicated fields.

**Demo Subsystem** (`UMRIDemoSubsystem`): A `UGameInstanceSubsystem` that manages demo recording, playback, replay enumeration, and frame rendering. Because it is a subsystem rather than a custom GameInstance, any project can use it without replacing their game instance class. It also handles navigation sequence loading, subject state persistence between sessions, and provides an OBS WebSocket integration stub for synchronized video recording.

**Spectator Controller** (`AMRISpectatorController`): The controller that takes over during demo replay. It attaches to the subject's recorded pawn, steps through the replay at a fixed timestep, triggers frame capture on all sensors each tick, and logs experiment state via the logger component. When the replay ends, it writes the accumulated per-frame state to an XML log file. Override `SpawnLoggerComponent()` to use an experiment-specific logger, and override `SaveExperimentLog()` to add custom sections to the XML output.

**Logging** (`MRIExperimentState`, `UMRILoggerComponent`): The experiment state class captures a snapshot of relevant experiment state for a single frame, from all entities and the subject's controls/TTL state. It then write it to a human-readable XML. The logger component is an actor component on the spectator controller that creates these snapshots each frame. Inherit from both to add experiment-specific data.

**Sensor** (`AMRIFrameCaptureCamera`): A scene capture camera actor that renders frames to PNG files on disk. It wraps a `USceneCaptureComponent2D` with a render target, supports configurable post-process effects (depth, semantic segmentation, normals), and provides auto-incrementing filename saves. The spectator controller calls `SaveNextFrame()` on each sensor every tick during replay rendering. Adapted from CARLA 0.8.4 (see Credits).

**Settings** (`UMRISettings`): Configuration management for the experiment. Reads and writes INI-format config files with settings for the experiment type, subject ID, auto-trigger/auto-stop behavior, eyetracking calibration, display resolution, TTLs before experiment start, and semantic segmentation. The settings object is created and owned by the Demo Subsystem. Adapted from CARLA 0.8.4 (see Credits).

**Agent** (`UMRIAgentComponent` and subclasses): A visitor-pattern entity typing system. Actors in the world that should be tracked during replay should have an agent component attached. During state logging, the experiment state visits each agent component to record its state information at each tick. An agent component needs to be added for each entity type you deal with in your experiment. Adapted from CARLA 0.8.4 (see Credits).

## Credits and Acknowledgements

The agent component / visitor pattern (`UMRIAgentComponent`, `IMRIAgentComponentVisitor`), the frame capture camera sensors (`AMRIFrameCaptureCamera`, `EMRIPostProcessEffect`), and the settings system (`UMRISettings`) were adapted from the [CARLA open-source autonomous driving simulator](https://github.com/carla-simulator/carla), version 0.8.4.

Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). Licensed under the [MIT License](https://opensource.org/licenses/MIT).

The demo recording and playback logic was implemented with reference to the [Unreal Engine community replay system tutorial](https://unrealcommunity.wiki/replay-system-tutorial-41kq5b58). 
