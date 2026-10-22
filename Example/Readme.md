# Example Project

The Example project provides a simple demonstration of how to use the Experiment plugin. 
The Third-person template is used to implement a very simple foraging task. The game records a demofile for the session, and provides a UI for playing it back to export an XML human-readable log and renders semantic segmentation frames.

## The task

This game implements a simple foraging task. On each trial, 1-3 random items are spawned on the map and the participant is instructed to collect them. The subject walks into a box to collect it gets one point per item collected. Trials end when the last item is collected, and a new trial starts ten seconds later. The HUD shows a timer counting the seconds since the trial started and a direction arrow that points toward the nearest remaining box.

To start a demo recording, use `Esc` to open the menu, and click the "Start Recording button". Trials start  after the configured number of TTL pulses has arrived ( default 5). The system is set up such that the first TTL starts the demo recording if the operator forgets to start one as a failsafe (when `AutoTriggerDemoRecording` is enabled). To end the demo record, open the menu and click "stop recording". The demo can the be selected and played back in the list of demos pages in the settings.

## Usage of Experiment plugin classes

Each class in the Example progject subclasses one class in the plugin and overrides its extension points.

**Controller**: `AExamplePlayerController` subclasses `AExperimentSubjectController`. The base class counts TTLs, manages auto-started demo recording, and provides the HUD display delegates. 
The Example controlled overrides `ExperimentTick()` to implement the experiment logic: trial countdown, box spawning on the navigation mesh, overlap-based collection, prompts, points, and the timer. 
It also binds movement input, the settings menu keys, and exposes `GetDirectionToNearestItem()` for the HUD arrow.

**State**: `AExampleSubjectState` subclasses `AExperimentSubjectState`. 
This class stores the subject's transform, TTL state, and points into the demo recording. 
(The forage items(`BP_TrialItem`) are replicated actors, so the demo records their spawning and collection without any extra task state.)

**Demo Subsystem**: the Example uses `UExperimentDemoSubsystem` as is. The settings menu calls it to enumerate demos on disk, play them back, and render them.

**Spectator Controller**: `AExampleSpectatorController` subclasses `AExperimentSpectatorController`. 
This controller is spawned when playing a demo, and drives replays at a fixed timestep and runs the capture and logging pipeline. 
The Example overrides `AttachCamera()` attach its own camera to the player camera from the recorded gameplay, `ConfigureSemanticLabels()` to define the segmentation labels, and `SpawnLoggerComponent()` to use its own logger. 

**Logging**: `ExampleExperimentState` and `UExampleLoggerComponent` subclass the plugin's logging pair. On each frame during the replay, the Logger component iterates through the world to create an ExperimentState that captures the state of the world at that time. 
At the end of the replay, this series of state objects is used to write out the XML log. These classes define the structure of the XML log file.

**Camera**: capture cameras come from the settings file (see below). The RGB camera saves what the subject saw; the SemSeg camera renders the semantic segmentation written by the plugin's tagger. 
The Example project labels the character 1, the trial boxes 2, the sky 4, and everything else 3 (see `ConfigureSemanticLabels` in the spectator controller); each label renders as a fixed, distinct color.

**Settings**: `Config/ExperimentSettings.ini` holds the per-session configuration: experiment type, subject ID, recording behavior, resolution, TTL count, HUD options, and the replay camera list. 
Each camera is defined in its own config section with a post-process effect, image size, and field of view.

## Records produced

During the experiment, the game proudes a demofile.

- **Demos** — `Saved/Demos/<name>.replay`, the engine-readble demofile the contains the state of the the entire world. This is used by the demo system to recreate the experiment offline. It cannot be parsed by external tools.

From the in-game menu, the user can play back the demo files at a fixed frame rate (default 15 fps). 
When playing back the demo files, the game produces two further sets of files. 
If this have been rendered, the in-game menu will mark the demo with "(R)". 
The fixed frame rate sets the temporal resolution of the XML and rendered frames. 
During playback, the game will lock its internal clock to this framerate and will only step the world clock after all rendering and logging has completed on each frame.


- **XML logs** — `Saved/<name>-log.xml`, a human/external tool-parsable file that records the state of the experiment. Produced by the Logger Compnent.

- **Rendered frames** — `Saved/Renders/<name>/<camera>/capture_NNNNNN.png`, one subfolder per configured camera. Each game frame saves out one image.