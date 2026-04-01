// MRI Experiment Plugin
// Framework for running human subjects experiments in MRI scanners using Unreal Engine.

#pragma once

#include "ModuleManager.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

/** Log category for all MRI Experiment plugin messages. */
DECLARE_LOG_CATEGORY_EXTERN(LogMRI, Log, All);

/**
 * A plugin containing logic for running MRI experiments in Unreal Engine.
 *
 * Implements logic useful for MRI experiments, including:
 * - replication and information extraction / re-rendering
 * - basic TTL handling and skeleton for experiment logic
 * - Settings I/O
 *
 * Many of the logic here are scaffolds for building experiments, and the user
 * will need to subclass and implement many of the virtual functions, and also
 * extend some enum definitions.
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
static FName PlayerTag = FName(TEXT("Player"));
/** Actor tag used to identify NPC (non-player character) pawns in the world. */
static FName NPCTag = FName(TEXT("NPC"));
