// Copyright Gallant Lab, UC Berkeley

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EyelinkGameModeBase.generated.h"

/**
 * Base game mode that exposes Eyelink eyetracking commands to Blueprints and the console.
 *
 * Provides Blueprint-implementable events for calibration, interrupt, and validation,
 * as well as native implementations for sending raw Eyelink commands and keycodes.
 * Subclass this game mode in projects that use Eyelink eyetracking.
 */
UCLASS()
class EYELINK_API AEyelinkGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	/**
	 * Initiates Eyelink eyetracking calibration.
	 * Can be triggered from the in-game console or called from Blueprints.
	 */
	UFUNCTION(Exec, BlueprintImplementableEvent, Category = ExecFunctions, BlueprintCallable)
	void EyelinkCalibrate();

	/**
	 * Interrupts the current Eyelink eyetracking session or calibration.
	 * Can be triggered from the in-game console or called from Blueprints.
	 */
	UFUNCTION(Exec, BlueprintImplementableEvent, Category = ExecFunctions, BlueprintCallable)
	void EyelinkInterrupt();

	/**
	 * Initiates Eyelink drift-correction validation.
	 * Can be triggered from the in-game console or called from Blueprints.
	 */
	UFUNCTION(Exec, BlueprintImplementableEvent, Category = ExecFunctions, BlueprintCallable)
	void EyelinkValidate();

	/**
	 * Sends a raw command string to the Eyelink host and returns its response.
	 * Logs both the command and the response via LogEyelink.
	 * @param command  The Eyelink command to send.
	 * @return The response string from the Eyelink host, or an error description if unavailable.
	 */
	UFUNCTION(Exec, Category = ExecFunctions, BlueprintCallable)
	FString EyelinkSendCommand(FString command);

	/**
	 * Sends a raw keycode to the Eyelink host.
	 * Useful for simulating key presses on the Eyelink host computer.
	 * @param keycode  The keycode to send to the Eyelink host.
	 */
	UFUNCTION(Exec, Category = ExecFunctions, BlueprintCallable)
	void EyelinkSendKey(int keycode);
};
