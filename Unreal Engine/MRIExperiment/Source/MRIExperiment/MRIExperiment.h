// MRI Experiment Plugin
// Framework for running human subjects experiments in MRI scanners using Unreal Engine.

#pragma once

#include "ModuleManager.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMRI, Log, All);

class FMRIExperimentModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

// Globally available tags for identifying subject and NPC pawns
static FName MRIPlayerTag = FName(TEXT("Player"));
static FName MRINPCTag = FName(TEXT("NPC"));
