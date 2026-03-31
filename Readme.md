## It's Complicated

\<Insert Avril Lavigne reference here>

This is a gross oversimplification, but traditional experiments are open-loop and parametric, which makes data collection and analysis relatively straightforward.

If you want to do closed loop naturalistic experiments that better reflect real-world situations, things suddenly get very complicated thanks to emergent complexity and chaos. But these experiments are more fun. However, traditional software platforms aren't appropriate for these experiments. This repo provides a set of tools and a description of the approach to implement things in a game engine.

##### What this is
This is a collection of tools collected from a bunch of different experiments and across time. They reflect an approach to running things in the MRI. These tools each simplify or make possible some aspect of complex closed-loop experiments.

##### What this isn't
This is not a suite of tools that lets you plop in a scientific idea and just run it. That's not feasible given how unique each situation is. These tools rather give you perspective on how to tackle the chaos that closed-loop/complex experiments create.

### Unreal Plugins
These plugins help you build an experiment from the ground-up in Unreal Engine. Obviously, you can lift the logic and apply it to other engines like Unity.
#### MRIExperiment
This is lifted out of the driving simulator. These things take care of the experiment logic framework in Unreal that makes all the things possible.

#### Eyelink
This is a plugin that will talk to an Eyelink system and display calibration stuff.


### Standalone Programs
These things are helpers for when you run your experiment.
#### [GameMonitor](https://github.com/gallantlab/gamemonitor)
A thing that will talk to Eyelink, log the state of a controller, and also start/stop OBS screen recordings

#### [SharpEyes](https://github.com/candytaco/SharpEyes)
Because eyetracking is messy, this lets you manually correct eyetracking stuff, and also has a model-free gaze mapping, if you have the raw videos and know where the calibration dots are.


### Python libraries
These are tools for taking your outputs and pulling numbers out of them so you can do science.
#### [Driving-utilities](https://github.com/candytaco/driving-utilities)
A thing that can deal with the frames rendered out by the game engine and make useful features out of them

#### [Demofiles](https://github.com/gallantlab/demofiles)
A thing to read demofiles put out by Source Engine-based games. This was started off by James, then I improved upon it, and then Circle Chen (undergrad who I was working with through Alane for the Portal 2 stuff) improved upon it some more.