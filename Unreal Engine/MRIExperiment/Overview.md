# MRIExperiment Plugin Overview

The MRIExperiment plugin provides a reusable framework for building human subjects experiments that run inside MRI scanners using Unreal Engine. It handles the core infrastructure that every MRI experiment needs: synchronizing with the scanner's TTL trigger pulses, recording and replaying demo files of each experimental run, rendering frames and logging per-frame experiment state from those replays, and managing experiment configuration. The plugin is designed so that experiment-specific logic lives in subclasses that override a small number of virtual methods, while the base classes handle all the plumbing.

The overall data flow is: the subject interacts with an Unreal world while the scanner sends TTL pulses that the Controller tracks. The Controller auto-triggers demo recording on the first TTL and calls `ExperimentTick()` once enough TTLs have arrived for the experiment to begin. Everything the subject does (position, controls, TTL state) is replicated via the State into the demo file. After the session, the Demo Subsystem plays back each recorded demo and spawns a Spectator Controller, which steps through the replay frame-by-frame at a fixed timestep, captures sensor images to disk via the Frame Capture Camera, and logs entity positions and subject state to an XML file via the Logger and Experiment State classes. Settings for all of this are read from an INI config file by the Settings module.

## Modules

**Controller** (`AMRISubjectController`): The player controller that the subject uses during an experiment. It binds to TTL input events from the scanner, counts TTLs until the experiment should start, manages auto-triggering and auto-stopping demo recording, runs experiment-specific tick logic, and provides a delegate-based system for displaying text prompts to the subject. Inherit from this class and override `ExperimentTick()` to implement your experiment.

**State** (`AMRISubjectState`): The replicated player state that travels with the subject's pawn. It stores the subject's transform, TTL state, points, and the currently displayed prompt type. All properties are replicated so they get recorded into demo files and are available during replay. Inherit from this to add experiment-specific replicated fields.

**Demo Subsystem** (`UMRIDemoSubsystem`): A `UGameInstanceSubsystem` that manages demo recording, playback, replay enumeration, and frame rendering. Because it is a subsystem rather than a custom GameInstance, any project can use it without replacing their game instance class. It also handles navigation sequence loading, subject state persistence between sessions, and provides an OBS WebSocket integration stub for synchronized video recording.

**Spectator Controller** (`AMRISpectatorController`): The controller that takes over during demo replay. It attaches to the subject's recorded pawn, steps through the replay at a fixed timestep, triggers frame capture on all sensors each tick, and logs experiment state via the logger component. When the replay ends, it writes the accumulated per-frame state to an XML log file. Override `SpawnLoggerComponent()` to use an experiment-specific logger, and override `SaveExperimentLog()` to add custom sections to the XML output.

**Logging** (`MRIExperimentState`, `UMRILoggerComponent`): The experiment state class captures a snapshot of relevant experiment state for a single frame, from all entities and the subject's controls/TTL state. It then write it to a human-readable XML. The logger component is an actor component on the spectator controller that creates these snapshots each frame. Inherit from both to add experiment-specific data.

**Sensor** (`AMRIFrameCaptureCamera`): A scene capture camera actor that renders frames to PNG files on disk. It wraps a `USceneCaptureComponent2D` with a render target, supports configurable post-process effects (depth, semantic segmentation, normals), and provides auto-incrementing filename saves. The spectator controller calls `SaveNextFrame()` on each sensor every tick during replay rendering. 
These are extended from the original CARLA implementations as of 0.8.4 - see CARLA for more information.

**Settings** (`UMRISettings`): Configuration management for the experiment. Reads and writes INI-format config files with settings for the experiment type, subject ID, auto-trigger/auto-stop behavior, eyetracking calibration, display resolution, TTLs before experiment start, and semantic segmentation. The settings object is created and owned by the Demo Subsystem.

**Agent** (`UMRIAgentComponent` and subclasses): A visitor-pattern entity typing system. Actors in the world that should be tracked during replay should have an agent component attached. During state logging, the experiment state visits each agent component to record its state information at each tick. An agent component needs to be added for each entity type you deal with in your experiment.

### Credits
Some of these systems (the agent component pattern, the camera sensors, and the settings system) were adapted from CARLA. the demo replay was implemented following a UE tutorial.