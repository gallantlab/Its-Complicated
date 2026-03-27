// MRI Experiment Plugin
// Framework for running human subjects experiments in MRI scanners using Unreal Engine.

#pragma once

#include "ModuleManager.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

/** Log category for all MRI Experiment plugin messages. */
DECLARE_LOG_CATEGORY_EXTERN(LogMRI, Log, All);

/**
 * Primary module class for the MRI Experiment plugin.
 *
 * Implements the IModuleInterface lifecycle callbacks so that Unreal Engine
 * can load and unload the plugin at the appropriate times.
 */
class FMRIExperimentModule : public IModuleInterface
{
public:
	/**
	 * Called immediately after the module is loaded into memory.
	 * Logs a confirmation message via LogMRI.
	 */
	virtual void StartupModule() override;

	/**
	 * Called before the module is unloaded during shutdown.
	 * Logs a confirmation message via LogMRI.
	 */
	virtual void ShutdownModule() override;
};

/** Actor tag used to identify the subject (player) pawn in the world. */
static FName MRIPlayerTag = FName(TEXT("Player"));
/** Actor tag used to identify NPC (non-player character) pawns in the world. */
static FName MRINPCTag = FName(TEXT("NPC"));
