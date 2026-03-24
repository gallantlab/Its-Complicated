# State Module

**Files:** `State/MRISubjectState.h`, `State/MRISubjectState.cpp`

## What It Does

`AMRISubjectState` inherits from `APlayerState` and stores everything about the subject thatyou eventually want to read out for your experiment. During live gameplay, the controller writes into these properties each tick, and because they are all marked `UPROPERTY(Replicated)`, the demo net driver captures them. During replay, the spectator controller reads them back out through the getter methods.

The replicated properties cover the subject's transform and motion,  MRI synchronization (TTL state, total TTL count, beep marker, seconds to start of run), experiment state (points, displayed prompt type, whether under player control, auto-steer status), and timing metadata (frame number, timestamps). You then need to extend all those classes to record whatever is relevant for you.

The class declares `AMRISubjectController` as a friend so the controller can write directly to the private properties without needing setters for every field.

## How to Use

Subclass `AMRISubjectState` for your experiment, and then set it as the `PlayerStateClass` in your game mode. The base controller handles writing all the standard properties. You do not need to manually update them.

To read subject state during replay, cast the reference pawn's `PlayerState` to your class, and read out the properties you implemented.

When subclassing, override `GetLifetimeReplicatedProps()` (call Super first), `CopyProperties()` (call Super first), and `ResetExperimentState()` (call Super first) to include your new properties. Remember to declare your experiment's controller as a friend class if it needs direct write access.
