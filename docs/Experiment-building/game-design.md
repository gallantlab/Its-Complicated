# Game engine concepts for experiments

This section provides an overview of some game design concepts for neuroscience experiments. It is not a comprehensive guide to building games, but provides a crash course on key concepts in game building that differ vastly from traditional experiment-building software packages. The understanding provided in this section should enable the reader then be able to learn game design with the intention of building neuroscience experiments. Because I work in Unreal Engine, this section will be heavily Unreal-biased, but should be easily translatable to Unity and other engines.

## Basic overview

The experiment, or game, takes place in a world. The world contains many static objects that form the backdrop and environment. This environment is populated by actors that interact with each other according to a set of rules. Each agent has a controller that implements the logic of the agent, and the agent can optionally embody a pawn, which is its physical manifestation in the world. The controller can switch between pawns as needed. Both human players and AI agents (or nonplayer characters, NPCs) interface with the world through controllers. NPC controllers contain logic trees for determining AI actions, while player controllers take user input and translate them to in-game actions. A virtual camera in the game world serves as the viewport that acts as the player’s eyes. A head-up display (HUD) can then be layered over the camera output to provide additional information.

## The Game World, Mode, and Instance

There are three main aspects to the world: the physical world in which the game occurs, the game mode that describes the rules of the game, and the game instance that is the actual instantiation of the game. The physical world is the environment in which actors interact with each other. For example, the world can be a 3D map of a city, or a 2D chess board. The game mode describes the objectives of the game and how actors interact with each other. For example, one game mode can describe a race between characters, while a second game mode can describe an Easter egg hunt. Finally, the game instance is the instantiation of the world: the world and game mode are Platonic descriptions, while the game instance is a unique embodiment of the game when the user starts the program. The game instance evolves over time as agents interact. The game world also loads other systems, such as physics simulation or networking, to perform other game function.

## Controllers and Pawns

The best way to understand the controller-pawn distinction is the philosophical idea of Dualism. The controller is the mind, and the pawn is the body. The pawn, e.g. a person, car, or robot, has a physical existence in the game world. The controller is the invisible puppeteer that moves the pawn around the world. This distinction separates the logic of interaction from the embodiment of the interaction. For example, when a player enters a vehicle, the player controller changes from controlling a “person” pawn to controlling a “car” pawn. The player can also switch between pawns; when the player controller leaves a pawn, an NPC controller can take over the pawn, and from the perspective of a third actor, the pawn has a single continuous existence in the world and continues to act.

A controller does not even need to possess a full embodied pawn. For example, a spectator does not need a physical body in the game world, and only has a camera.

## Cameras and displays

The player sees the world through an in-world camera. This camera is typically attached to the pawn controlled by the player. In a first-person game, this camera is on the pawn. In a third-person perspective, this camera follows the pawn from a distance. This camera outputs a video stream for the player. Then, in order to display more information to the player, an HUD can be overlaid on this camera output. The HUD can contain information such as map information, current goals, points accrued, etc. The key distinction between the in-world camera and the HUD is that the in-world camera is a 3D camera, while the HUD is a 2D display. The in-world camera operates according to the laws of optics and projection, while the HUD draws pixel values. The player controller needs to explicitly display information to the HUD, while the in-world camera is attached the to pawn and there is no direct interaction between the controller and the camera.

## Time and experiment logic

Perhaps the biggest difference between a game engine and traditional experiment tools is the passage of time. In tradition experiments, there is always a “main” loop that progresses the experiment. This loop is typically run at a fixed refresh rate, and all state information is updated in this loop. In a game engine, this loop is abstracted away from the end-user. Instead, the world clock simply ticks, and each actor, on each tick, is informed of the passage of time, and acts accordingly.

Experiment logic can be implemented in many aspects of the game. For example, an NPC agent can move away from the player, the player’s controller can display the distance to the NPC, and the game instance can track the number of foraging targets. It is important to note that these logic components are compartmentalized within each actor in the world, and are updated separated as the world clock ticks.

## Inputs

In traditional experiment tools, user input is often tied explicitly to the human interface device (HID). For example, pressing the “1” key can be mapped to “yes” and the “2” key can be mapped to “no.” In game engines, the HID input and game actions are separated. At the game design level, the experimenter can specify an arbitrary set of actions, such as “yes” and “no,” and describe what HID inputs can be mapped to these actions. In the game, the controllers receive the “yes” and “no” actions and act accordingly. Doing so enables user interaction to be more flexible, and the inputs can be easily remapped during runtime.

## Replays

In the previous page we had described the basic concept of a replication and the replays it provides. When designing an experiment, the experiment marks variables to be replicated. Note that many game variables are already replicated by default; the experiment only needs to mark their custom game variables. To extract information from a replay, a spectator actor queries the replay at each timestep, and extracts the relevant information. These can be any variable, such as position information or a whole new camera in the world that views the scene from a certain perspective.

## Example code

In the accompanying code we provide an Unreal Engine plugin that implements many of these core functions. In future updates we will include an example project that will demonstrate basic usage of this code.
