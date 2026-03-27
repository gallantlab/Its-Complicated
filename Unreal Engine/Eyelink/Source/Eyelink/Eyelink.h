// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"

/** Log category for all Eyelink plugin messages. */
DECLARE_LOG_CATEGORY_EXTERN(LogEyelink, Log, All);

/**
 * A plugin that interfaces with an Eyelink via the C api
 *
 * The Eyelink SDK make strong assumptions that the user will be writing linear programs, 
 * which does _not_ match with how we make complex, object-oriented modern programs, esp. in Unreal. 
 * This plugin wraps the Eyelink C library in UEyelinkInterface, which exposes basic Eyelink functions
 * as delegates and Blueprint-callable functions. Because the Eyelink C library is not designed 
 * to be initialized more than once per process, the module/Interface should be a singleton to
 * keeping the interface alive for the full GameInstance lifecycle.
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