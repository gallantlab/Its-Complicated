# Camera Module

**Files:** `Camera/ExperimentFrameCaptureCamera.h`, `Camera/ExperimentFrameCaptureCamera.cpp`, `Camera/ExperimentPostProcessEffect.h`, `Camera/SemSegTagger.h`, `Camera/SemSegTagger.cpp`

**Attribution:** `AExperimentFrameCaptureCamera` and `EExperimentPostProcessEffect` were adapted from the [CARLA open-source autonomous driving simulator](https://github.com/carla-simulator/carla), version 0.8.4. Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). Licensed under the [MIT License](https://opensource.org/licenses/MIT).

## What It Does

`AExperimentFrameCaptureCamera` is an actor that wraps a `USceneCaptureComponent2D` and a `UTextureRenderTarget2D` to capture rendered frames and save them as PNG files to disk. It supports multiple post-process effects defined by the `EExperimentPostProcessEffect` enum: SceneFinal (normal RGB), Depth, SemanticSegmentation, WorldNormal, and CameraNormal. The post-process materials are loaded by path in the constructor; projects should assign the correct material references for their setup.

The camera has configurable image size, field of view, and a captures-per-second rate for timed capture mode. For Experiment replay rendering, the spectator controller calls `SaveNextFrame()` each tick, which captures the current scene, encodes it as PNG using `IImageWrapper`, and writes it to the save folder with an auto-incrementing filename (e.g., `image0000.png`, `image0001.png`, ...). `SetSaveFolder()` configures the output directory and `AttachToActor()` attaches the camera to the subject's pawn so it captures from their viewpoint.

## How to Use

List the cameras you want in the settings file (see the Settings module): one entry per image output, each with its post-process effect, image size, and FOV. The spectator controller spawns them at replay start, attaches them through its `AttachCamera()` override point, and gives each one a subfolder of the capture folder named after the camera. `CaptureCameras()` then calls `SaveNextFrame()` on every camera each tick.

For standalone use outside of replay rendering, set `bCaptureScene = true` and configure `CapturesPerSecond` for automatic timed capture in the `Tick` function.

To get different effects, you should add additional PostProcessEffects.

## Semantic Segmentation Tagging

`USemSegTagger` writes a per-actor label into the custom depth stencil buffer, which a post-process material on a semantic segmentation camera maps to colors. The label map assigns a stencil value to each actor class; when several entries match an actor, the most derived class wins. Tagging sets the stencil value on all of the actor's primitive components and enables the custom depth pass on them, which renders each tagged object a second time per frame.

Override the spectator controller's `ConfigureSemanticLabels()` to fill its `semanticLabels` class map, `semanticTagLabels` actor-tag map, and `defaultSemanticLabel`. When `CreateCameras()` registers a camera with the SemanticSegmentation effect, the spectator controller calls it, tags the whole level, and registers a spawn handler so actors that appear mid-replay are tagged as well. Without such a camera, no tagging runs and the custom depth pass stays off.
