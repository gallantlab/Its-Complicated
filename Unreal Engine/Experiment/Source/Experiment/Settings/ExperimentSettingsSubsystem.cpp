// Copyright (c) Gallant Lab. All Rights Reserved.

#include "ExperimentSettingsSubsystem.h"

void UExperimentSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	settings = NewObject<UExperimentSettings>(this, TEXT("ExperimentSettings"));
	check(settings != nullptr);
	settings->LoadSettings();
	settings->LogSettings();

	UE_LOG(LogExperiment, Log, TEXT("ExperimentSettingsSubsystem initialized"));
}

void UExperimentSettingsSubsystem::Deinitialize()
{
	SaveSubjectStateFile();
	Super::Deinitialize();
}

UExperimentSettings& UExperimentSettingsSubsystem::GetSettings()
{
	check(settings != nullptr);
	return *settings;
}
