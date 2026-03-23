## It's Complicated

\<Insert Avril Lavigne reference here>

This is a gross oversimplification, but traditional experiments are input-driven and open loop, which makes data collection and analysis relatively straight forward.

If you want to do closed loop, input-output experiments, things get suddenly very complicated thanks to emergent complexity and chaos. But these experiments are more fun. But if you try to run it all by hand or in the traditional ways, you'll end up like Spongebob

![](Images/spongebob.gif)

To make these experiments more tractable, I've made variaous tools on and off. This repo collects all of them here.

##### What this is
This is a collection of tools collected from a bunch of different experiments and across time. They reflect an approach to running things in the MRI. These tools each all simplify or make possible some aspect of complex closed-loop experiments.

##### What this isn't
This is not a suite of tools that lets you plop in a scientific idea and just run it. That's... not feasible given how each unique situation is. 

In fact, some of these tools intentially will not compile, or will segfault if you try to run them out of the box. These tools rather give you perspective on how to tackle the chaos that closed-loop/complex experiments create.


### Unreal Plugins
#### MRIExperiment
This is lifted out of the driving simulator. These things take care of the experiment logic framework in Unreal that makes all the things possible.

#### Eyelink
This is a plugin that will talk to an Eyelink system and display calibration stuff.


### Standalone Programs
#### GameMonitor
A thing that will talk to Eyelink, log the state of a controller, and also start/stop OBS screen recordings

#### SharpEyes
Because eyetracking is messy, this lets you manually correct eyetracking stuff, and also has a model-free gaze mapping, if you have the raw videos and know where the calibration dots are.


### Python libraries
#### Driving-utilities
A thing that can deal with the frames rendered out by the game engine and make useful features out of them

#### Demofiles
A thing to read demofiles put out by Source Engine-based games