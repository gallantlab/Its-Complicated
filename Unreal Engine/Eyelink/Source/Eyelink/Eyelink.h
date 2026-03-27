// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"

/** Log category for all Eyelink plugin messages. */
DECLARE_LOG_CATEGORY_EXTERN(LogEyelink, Log, All);

/**
 * Primary module class for the Eyelink plugin.
 *
 * Implements the IModuleInterface lifecycle callbacks so that Unreal Engine
 * can load and unload the plugin at the appropriate times.
 */
class FEyelinkModule : public IModuleInterface
{
public:

	/**
	 * Called immediately after the module is loaded into memory.
	 * Use this to perform any one-time initialization required by the module.
	 */
	virtual void StartupModule() override;

	/**
	 * Called before the module is unloaded during shutdown (or dynamic reload).
	 * Use this to clean up any resources allocated during StartupModule.
	 */
	virtual void ShutdownModule() override;
};