# Controller Module

**Files:** `Controller/MRISubjectController.h`, `Controller/MRISubjectController.cpp`

## What It Does

`AMRISubjectController` is the player controller used by the subject during an MRI experiment. It inherits directly from `APlayerController`, so it is not tied to any particular pawn type (vehicles, characters, etc.).

The controller's main job is managing the experiment lifecycle around the scanner's TTL pulses. When the scanner sends a TTL (mapped to the "TTL" input action, here to the 5 key), the controller's `TTLdown()` fires. On the very first TTL, it optionally auto-triggers demo recording. It then counts TTLs until `TTLsToExperimentStart` have arrived (default 5, configurable in settings), after which it starts calling `ExperimentTick()` every frame. If no TTL arrives for `SecondsToDemoStop` seconds, it auto-stops the demo recording.

The controller also provides a display text system. Call `SetDisplayText()` with a string, duration, prompt type, and color, and the controller will fire delegates that a UI widget can bind to. It automatically clears the text after the duration expires and calls `OnEndDisplayText()`.

On `BeginPlay`, the controller reads settings from `UMRIDemoSubsystem::GetSettings()` to configure auto-trigger, auto-stop, eyetracking, TTL count, and resolution. It accesses the demo subsystem via `UGameInstance::GetSubsystem<UMRIDemoSubsystem>()`.

## How to Use

Create a subclass of `AMRISubjectController` for your experiment. Override `ExperimentTick(float deltaTime)` to run your experiment logic each frame after the TTL countdown completes. Override `TTLdown()` and `TTLup()` if you need custom behavior on each TTL pulse (call `Super` to keep the base counting and auto-trigger logic). Override `ResetExperimentState()` to clean up when a demo recording ends, and `OnEndDisplayText()` to react when a timed prompt finishes displaying.

To display prompts to the subject, call `SetDisplayText("Your text", duration, EDisplayedPromptType::YourType)`. Bind a UMG widget to `SetDisplayTextDelegate` and `SetDisplayTextColorDelegate` to render these on screen.

Set your experiment's controller as the default player controller in your game mode.

Things to override
- `ExperimentTick(float deltaTime)` -- called every frame once the TTL countdown is satisfied and a demo is recording. Put your experiment logic here.
- `TTLdown()` / `TTLup()` -- called on each TTL pulse edge. Call Super to preserve base behavior.
- `ResetExperimentState()` -- called when demo recording ends. Reset your experiment-specific variables here.
- `OnEndDisplayText()` -- called when a timed display text prompt expires. Override to trigger follow-up actions.
- `UpdatePoints(int increment)` -- override to customize point scoring behavior.
