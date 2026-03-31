# Tools for game engine experiments

## Unreal Engine Plugins
These plugins provide infrastructure for designing MRI experiments in Unreal Engine

### MRIExperiment

A plugin that provides the basic scaffolding for MRI experiments in Unreal Engine. Key functionalities include handling TTLs, running the replay system, and logging. See the [MRIExperiment plugin reference](../Unreal-Plugins/MRIExperiment) for the full API.

#### Credits and Acknowledgements

The agent component / visitor pattern, the frame capture camera sensors, and the settings system were adapted from [CARLA](https://github.com/carla-simulator/carla) v0.8.4. Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). Licensed under the [MIT License](https://opensource.org/licenses/MIT).

The demo recording and playback logic was implemented with reference to the [Unreal Engine community wiki replay tutorial](https://unrealcommunity.wiki/replay-system-tutorial-41kq5b58).

### Eyelink

A plugin for interfacing with an Eyelink system from inside an Unreal project. See the [Eyelink plugin reference](../Unreal-Plugins/Eyelink/) for the full API.

## Standalone Programs

These programs run alongside the experiment, separate from the game engine process.

### GameMonitor

A standalone program that runs in the background to an experiment. It talks to Eyelink, logs raw gamepad readouts, and starts/stops OBS screen recordings.

External link: [https://github.com/gallantlab/gamemonitor](https://github.com/gallantlab/gamemonitor)

### SharpEyes
A tool for eyetracking. It finds pupils in raw eye video recordings, and also allows the user to view the fixation overlaid on the stimulus, and to manually correct eyetracking errors.

External link: [https://github.com/candytaco/SharpEyes](https://github.com/candytaco/SharpEyes)

## Python Libraries

These libraries operate on the data produced by the above tools to extract features for analysis.

### Driving-utilities
A Python library that reads outputs from the driving simulator to produce features for analysis.

External link: [https://github.com/candytaco/driving-utilities](https://github.com/candytaco/driving-utilities)

### Demofiles
A Python library to parse the recordings produced by Source Engine games.

External link: [https://github.com/gallantlab/demofiles](https://github.com/gallantlab/demofiles)

## Miscellaneous tools that are not directly experiment-related

### Unreal Doxygen
A package for generating documentation for Unreal C++. It includes a preprocessor parser to take care of Unreal Macros for Doxygen, and a tool to translate the Doxygen XML outputs to clean markdown for making a static site through Zensical. Used for the generating the Unreal Plugins API documentation in this repo.

External link :[https://github.com/candytaco/Unreal-Doxygen](https://github.com/candytaco/Unreal-Doxygen)