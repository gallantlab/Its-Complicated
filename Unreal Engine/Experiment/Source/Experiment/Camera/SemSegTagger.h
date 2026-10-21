// Copyright (c) Gallant Lab. All Rights Reserved.
//
// Portions of this file are adapted from the CARLA open-source autonomous driving simulator,
// version 0.8.4 (https://github.com/carla-simulator/carla).
// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// Licensed under the MIT License (https://opensource.org/licenses/MIT).

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "UObject/NoExportTypes.h"
#include "SemSegTagger.generated.h"

/**
 * Writes semantic segmentation labels into the custom depth stencil buffer.
 *
 * The label map assigns a stencil value to each actor class. Tagging an actor
 * sets that value on all of its primitive components and enables the custom
 * depth pass on them; a post-process material on a frame capture camera then
 * maps each pixel's stencil value to a color. The custom depth pass renders
 * every tagged object a second time each frame, so tag only when a
 * semantic segmentation camera is in use.
 */
UCLASS()
class EXPERIMENT_API USemSegTagger : public UObject
{
	GENERATED_BODY()

public:

	USemSegTagger();

	/**
	 * Replaces the class-to-label map used to tag actors.
	 * @param newLabelMap  Map from actor class to stencil value. Value 0 means untagged.
	 */
	void SetLabelMap(const TMap<TSubclassOf<AActor>, uint8>& newLabelMap);

	/**
	 * Tags every actor currently in the world.
	 * @param world  The world whose actors to tag.
	 */
	void TagActorsInLevel(UWorld& world) const;

	/**
	 * Tags one actor: writes its label into the custom depth stencil value of
	 * every primitive component and enables the custom depth pass when the
	 * label is not 0.
	 * @param actor  The actor to tag.
	 */
	void TagActor(AActor& actor) const;

	/**
	 * Registers a world spawn handler so actors that spawn later, such as
	 * replicated actors appearing mid-replay, are tagged as well.
	 * @param world  The world to watch for spawns.
	 */
	void RegisterSpawnHandler(UWorld* world);

private:

	/**
	 * Returns the label for the actor. When several map entries match, the
	 * most derived class wins, so a subclass entry overrides its parent's.
	 * @param actor  The actor to look up.
	 * @return The stencil value, or 0 when no entry matches.
	 */
	uint8 GetLabel(const AActor& actor) const;

	/** Spawn-handler callback that tags the new actor. */
	void OnActorSpawned(AActor* actor);

	/** Actor class to stencil value. */
	UPROPERTY()
	TMap<TSubclassOf<AActor>, uint8> labelMap;

	/** Delegate bound to OnActorSpawned, registered with the world by RegisterSpawnHandler. */
	FOnActorSpawned::FDelegate actorSpawnedDelegate;
};
