# Sensor Module

**Files:** `Sensor/MRIFrameCaptureCamera.h`, `Sensor/MRIFrameCaptureCamera.cpp`, `Sensor/MRIPostProcessEffect.h`

**Attribution:** `AMRIFrameCaptureCamera` and `EMRIPostProcessEffect` were adapted from the [CARLA open-source autonomous driving simulator](https://github.com/carla-simulator/carla), version 0.8.4. Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). Licensed under the [MIT License](https://opensource.org/licenses/MIT).

## What It Does

`AMRIFrameCaptureCamera` is an actor that wraps a `USceneCaptureComponent2D` and a `UTextureRenderTarget2D` to capture rendered frames and save them as PNG files to disk. It supports multiple post-process effects defined by the `EMRIPostProcessEffect` enum: SceneFinal (normal RGB), Depth, SemanticSegmentation, WorldNormal, and CameraNormal. The post-process materials are loaded by path in the constructor; projects should assign the correct material references for their setup.

The camera has configurable image size, field of view, and a captures-per-second rate for timed capture mode. For MRI replay rendering, the spectator controller calls `SaveNextFrame()` each tick, which captures the current scene, encodes it as PNG using `IImageWrapper`, and writes it to the save folder with an auto-incrementing filename (e.g., `image0000.png`, `image0001.png`, ...). `SetSaveFolder()` configures the output directory and `AttachToActor()` attaches the camera to the subject's pawn so it captures from their viewpoint.

## How to Use

During replay setup in the spectator controller, create one `AMRIFrameCaptureCamera` per sensor you want (e.g., one for RGB, one for depth, one for semantic segmentation). For each, call `SetImageSize()`, `SetPostProcessEffect()`, `SetFOVAngle()`, `AttachToActor()` with the subject pawn, and `SetSaveFolder()`. Add them to the spectator's sensor array so `CaptureSensors()` will call `SaveNextFrame()` on each one every tick.

For standalone use outside of replay rendering, set `bCaptureScene = true` and configure `CapturesPerSecond` for automatic timed capture in the `Tick` function.

To get different effects, you should add additional PostProcessEffects.
