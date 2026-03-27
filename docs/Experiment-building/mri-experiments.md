# Complex MRI Experiments vs Traditional Experiments

## Overview

[Placeholder: introduce the conceptual distinction between traditional open-loop experiments and closed-loop/complex experiments in the context of fMRI research.]

## Traditional (Open-Loop) Experiments

In a traditional fMRI experiment the stimulus is fixed ahead of time and delivered to the subject independent of anything the subject does. The experimenter has full control over what is shown and when, the data are structured and predictable, and analysis pipelines can be written against a known trial structure.

[Placeholder: expand on properties of traditional experiments — fixed stimulus sequences, blocked or event-related designs, straightforward GLM analysis, easy alignment of stimulus timing with TR.]

### Characteristics

- Stimulus is pre-computed and played back identically for every subject or run.
- No feedback loop between subject behavior and stimulus content.
- Trial structure is known at design time and can be embedded directly into the analysis model.
- Timing is controlled entirely by the experimenter through TTL-gated presentation.

[Placeholder: add examples from the literature and from this lab's own work.]

## Complex (Closed-Loop) Experiments

[Placeholder: describe what makes an experiment "complex" or "closed-loop." The stimulus changes in real time as a function of the subject's behavior, physiological state, or model predictions. This introduces emergent complexity because the data-generating process is no longer separable from the subject.]

### Why Things Get Complicated

[Placeholder: chaos, emergent complexity, the observer effect in interactive tasks. Why you cannot just pre-compute the design matrix.]

### Practical Challenges

[Placeholder: synchronization between the game engine and the scanner, handling variable trial lengths, logging sufficient state to reconstruct the stimulus post-hoc, dealing with latency.]

## How This Toolset Addresses These Challenges

[Placeholder: brief forward pointer to the tools. The MRIExperiment Unreal plugin solves the synchronization, logging, and replay problems. SharpEyes and the Python libraries solve the post-hoc analysis problems.]
