# Settings Module

**Files:** `Settings/ExperimentSettings.h`, `Settings/ExperimentSettings.cpp`

**Attribution:** The settings loading/saving pattern in this module was adapted from the [CARLA open-source autonomous driving simulator](https://github.com/carla-simulator/carla). Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). Licensed under the [MIT License](https://opensource.org/licenses/MIT).

## What It Does

`UExperimentSettings` is a `UObject` that manages experiment configuration through INI-format files. It reads settings from `Config/ExperimentSettings.ini` in the project directory or from a file path specified on the command line (via `-experiment-settings=`), and can write settings back out. `UExperimentSettingsSubsystem` creates and owns the settings object on initialization.

The settings are organized into a `[Experiment]` section in the INI file with fields for: `ExperimentType` (parsed by `UExperimentType::FromString()`), `Subject` (string identifier), `AutoTriggerDemoRecording` (bool), `AutoEyetrackingCalibration` (bool), `SecondsToDemoStop` (float, 0 to disable), `Resolution` (string like "1280x800w"), `RenderAll` (int, 0 to disable auto-rendering), `TTLsBeforeExperimentStart` (int, default 5), `ShowFPS` (bool), `ReticuleType` (string, e.g. "Multiply"), `Cameras` (comma-separated camera names).

Each name in `Cameras` has its own `[Experiment/Camera/<Name>]` section with `PostProcessing` (e.g. "SceneFinal", "SemanticSegmentation", "Depth"), `ImageSizeX`, `ImageSizeY`, and `FOV`. The spectator controller creates one frame-capture camera per entry during replay rendering, and each camera writes its frames into a subfolder named after it.

`EExperimentType` is an enum with all the experiment types, which you need to populate with what you will implement. The `UExperimentType` helper class provides `FromString()` and `ToString()` for converting between the enum and its string representation in config files, which you need to update along with the enum.

## How to Use

Access settings through the settings subsystem: `GetGameInstance()->GetSubsystem<UExperimentSettingsSubsystem>()->GetSettings()`. Your experiment controller should read the relevant values during its `BeginPlay`. For Blueprint access, use `GetExperimentSettings()` on the subsystem.

To add a custom config file, place it in your project's Config directory and pass its path on the command line. The settings object will read the `[Experiment]` section from that file.

`SetResolution()` provides a programmatic way to change the display resolution, formatting it as the `WIDTHxHEIGHTf` or `WIDTHxHEIGHTw` string that Unreal's `r.setRes` console command expects.

Your experiment configs would go in here. You can add new fields (you will also need to update the loading/saving methods) directly, or add a separate settings object in your experiment plugin that reads from its own INI section. 