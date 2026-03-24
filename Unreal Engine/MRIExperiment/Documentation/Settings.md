# Settings Module

**Files:** `Settings/MRISettings.h`, `Settings/MRISettings.cpp`

## What It Does

`UMRISettings` is a `UObject` that manages experiment configuration through INI-format files. It reads settings from the default game config file or from a file path specified on the command line (via `-carla-settings=`), and can write settings back out. The demo subsystem creates and owns the settings object on initialization.

The settings are organized into a `[fMRI]` section in the INI file with fields for: `ExperimentType` (parsed by `UExperimentType::FromString()`), `Subject` (string identifier), `AutoTriggerDemoRecording` (bool), `AutoEyetrackingCalibration` (bool), `SecondsToDemoStop` (float, 0 to disable), `Resolution` (string like "1280x800w"), `RenderAll` (int, 0 to disable auto-rendering), `TTLsBeforeExperimentStart` (int, default 5),  `SemanticSegmentationEnabled` (bool).

`EExperimentType` is an enum with all the experiment types, which you need to populate with what you will implement. The `UExperimentType` helper class provides `FromString()` and `ToString()` for converting between the enum and its string representation in config files, which you need to update along with the enum.

## How to Use

Access settings through the demo subsystem: `demoSubsystem->GetSettings()`. Your experiment controller should read the relevant values during its `BeginPlay`. For Blueprint access, use `GetMRISettings()` on the subsystem.

To add a custom config file, place it in your project's Config directory and pass its path on the command line. The settings object will read the `[fMRI]` section from that file.

`SetResolution()` provides a programmatic way to change the display resolution, formatting it as the `WIDTHxHEIGHTf` or `WIDTHxHEIGHTw` string that Unreal's `r.setRes` console command expects.

Your experiment configs would go in here. You can add new fields (you will also need to update the loading/saving methods) directly, or add a separate settings object in your experiment plugin that reads from its own INI section. 