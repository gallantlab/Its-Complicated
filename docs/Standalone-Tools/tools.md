# Tools Overview

This page summarizes all tools in the collection and their role in a complex MRI experiment workflow.

## Unreal Engine Plugins
These plugins provide infrastructure for designing MRI experiments in Unreal Engine

### MRIExperiment

[Placeholder: one-paragraph summary. The experiment logic framework for Unreal. Handles TTL synchronization, demo recording/replay, per-frame state logging, sensor frame capture, and experiment configuration. See the [MRIExperiment plugin reference](Unreal-plugins/mri-experiment-plugin/index.md) for the full API.]

### Eyelink

[Placeholder: one-paragraph summary. A plugin that bridges the SR Research Eyelink C API with Unreal Engine so that eyetracking calibration and data collection can be triggered from Blueprint or C++ during an experiment. See the [Eyelink plugin reference](Unreal-plugins/eyelink-plugin/index.md) for the full API.]

## Standalone Programs

These programs run alongside the experiment, separate from the game engine process.

### GameMonitor

[Placeholder: summary of GameMonitor. Talks to Eyelink, logs controller state, and starts/stops OBS screen recordings. Useful for monitoring the subject's eye position and input in real time from the control room without interfering with the game process.]

External link: [https://github.com/gallantlab/gamemonitor](https://github.com/gallantlab/gamemonitor)

### SharpEyes

[Placeholder: summary of SharpEyes. Post-hoc eyetracking correction tool. Because raw eyetracking data from MRI environments is noisy (gradient artifacts, head motion), SharpEyes lets you manually correct gaze traces and apply a model-free gaze mapping calibration if you have the raw eye videos and know the calibration dot positions.]

External link: [https://github.com/candytaco/SharpEyes](https://github.com/candytaco/SharpEyes)

## Python Libraries

These libraries operate on the data produced by the above tools to extract features for analysis.

### Driving-utilities

[Placeholder: summary. Takes frames rendered out by the game engine (via `AMRIFrameCaptureCamera`) and produces useful visual features — such as optical flow, depth estimates, and semantic maps — for use as regressors in the fMRI analysis.]

External link: [https://github.com/candytaco/driving-utilities](https://github.com/candytaco/driving-utilities)

### Demofiles

[Placeholder: summary. Reads `.dem` demofiles produced by Source Engine games (Half-Life 2, Portal 2). Useful for experiments that ran in a Source-based game rather than Unreal. The library was started by James, extended by T.Zhang, and further improved by Circle Chen.]

External link: [https://github.com/gallantlab/demofiles](https://github.com/gallantlab/demofiles)

## Misc stuff that are not directly experiment-related

### Unreal Doxygen
A package for generating documentation for Unreal C++. It includes a preprocessor parser to take care of Unreal Macros for Doxygen, and a tool to translate the Doxygen XML outputs to clean markdown for making a static site through Zensical. Used for the generating the Unreal Plugins API documentation in this repo.

## Workflow Summary

[Placeholder: a diagram or step-by-step description of how all tools connect. Data collection (MRIExperiment + Eyelink + GameMonitor) → raw outputs (demo files, eye data, screen recordings) → post-processing (SharpEyes, Driving-utilities, Demofiles) → analysis regressors.]
