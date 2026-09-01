// Copyright (c) Gallant Lab. All Rights Reserved.

#include "MRIExperiment.h"
#include "Settings/MRISettingsSubsystem.h"

void UMRISettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	settings = NewObject<UMRISettings>(this, TEXT("MRISettings"));
	check(settings != nullptr);
	settings->LoadSettings();
	settings->LogSettings();

	UE_LOG(LogMRI, Log, TEXT("MRISettingsSubsystem initialized"));
}

void UMRISettingsSubsystem::Deinitialize()
{
	SaveSubjectStateFile();
	Super::Deinitialize();
}

UMRISettings& UMRISettingsSubsystem::GetSettings()
{
	check(settings != nullptr);
	return *settings;
}
