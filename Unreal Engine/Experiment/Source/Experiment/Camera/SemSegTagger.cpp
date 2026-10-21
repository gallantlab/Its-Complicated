// Copyright (c) Gallant Lab. All Rights Reserved.
//
// Portions of this file are adapted from the CARLA open-source autonomous driving simulator,
// version 0.8.4 (https://github.com/carla-simulator/carla).
// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// Licensed under the MIT License (https://opensource.org/licenses/MIT).

#include "SemSegTagger.h"

#include "Experiment.h"
#include "Components/PrimitiveComponent.h"
#include "EngineUtils.h"

USemSegTagger::USemSegTagger()
	: actorSpawnedDelegate(FOnActorSpawned::FDelegate::CreateUObject(this, &USemSegTagger::OnActorSpawned))
{
}

void USemSegTagger::SetLabelMap(const TMap<TSubclassOf<AActor>, uint8>& newLabelMap)
{
	labelMap = newLabelMap;
}

void USemSegTagger::TagActorsInLevel(UWorld& world) const
{
	for (TActorIterator<AActor> it(&world); it; ++it)
		TagActor(**it);
}

void USemSegTagger::TagActor(AActor& actor) const
{
	const uint8 label = GetLabel(actor);
	if (label != 0)
		UE_LOG(LogExperiment, Log, TEXT("Tagged %s with stencil value %d"), *actor.GetName(), label);

	TArray<UPrimitiveComponent*> components;
	actor.GetComponents<UPrimitiveComponent>(components);
	for (UPrimitiveComponent* component : components)
	{
		component->SetCustomDepthStencilValue(label);
		component->SetRenderCustomDepth(label != 0);
	}
}

uint8 USemSegTagger::GetLabel(const AActor& actor) const
{
	const UClass* bestMatch = nullptr;
	uint8 bestLabel = 0;

	for (const TPair<TSubclassOf<AActor>, uint8>& entry : labelMap)
	{
		const UClass* entryClass = entry.Key.Get();
		if (entryClass && actor.IsA(entryClass))
		{
			if (!bestMatch || entryClass->IsChildOf(bestMatch))
			{
				bestMatch = entryClass;
				bestLabel = entry.Value;
			}
		}
	}

	return bestLabel;
}

void USemSegTagger::RegisterSpawnHandler(UWorld* world)
{
	if (world)
		world->AddOnActorSpawnedHandler(actorSpawnedDelegate);
}

void USemSegTagger::OnActorSpawned(AActor* actor)
{
	if (actor)
		TagActor(*actor);
}
