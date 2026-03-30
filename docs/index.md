---
icon: lucide/gamepad-2
---
# It's Complicated

## A guide for complex closed-loop experiments in the MRI

In the past 20 years these have been a revoltuion in human neuroimaging to use naturalistic stimuli to better reflect real-world experiences in the laboratory. The next logical frontier in this evolution are naturalistic tasks that are complex and interactive. However, naturalistic tasks necessarily imply closed-loop interactivity in which the participant influences the stimulus. Doing so creates emergent complexity and chaos (in the mathematical butterflies sense) that makes such experiments seem impossible to analyze. This problem for analysis is two-fold. First, traditional analysis method can't cope with the necessarily correlated structures of naturalistic tasks. Second, even extracting the information to analyze from these experiments may even seem impossible.

Banded ridge regression (Dupre la Tour et al. 2022, Nunez-Elizalde et al. 2019) provides a mathematical framework for dealing with these correlated features. However, to do banded ridge regression presumes that you have a design matrix on hand to fit to the data. How do you even get to the design matrix in the first place? You cannot control the participants' experience in interactive tasks in the way that you can control a stimulus.

Drawing from our experience of building a driving simulator and running navigation experiments (Zhang et al. 2025), this repository provides a guide for designing complex experiments for fMRI (and other neuroscience areas). It provides the other half of the solution to the chaos of naturalistic tasks, and gets you to the point of having design matrices to feed to models. It is largely focused on using games and game engines, but the concepts (and the standalone tools) can be applied to any experimental framework (given that the framework is capable of complexity).

The methods described here is still orders of magnitude more complicated than an experiment in, say, PsychoPy. What this guide and codebase does, however, is to turn what might seem impossible into simply complicated.

This guide is composed of a few parts:

#### [The programming paradigm and architecture for building such experiments](Experiment-building/Experiment-Concepts.md)
#### [Concepts from game engine for MRI experiments](Experiment-building/game-design.md)
#### [Some standalone tools that we have developed over the years to enable these experiments](Experiment-building/tools.md)
#### [Unreal Engine plugins that implements the overall paradigm and the API reference for this plugin](Unreal-Engine/index.md)
