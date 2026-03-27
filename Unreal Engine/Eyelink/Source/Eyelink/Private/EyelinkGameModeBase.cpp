// Copyright Gallant Lab, UC Berkeley

#include "EyelinkGameModeBase.h"

#include "Eyelink.h"
#include "EyelinkGameInstance.h"
#include "EyelinkInterface.h"


/**
 * Sends a raw command to the connected Eyelink device and returns its response.
 *
 * Retrieves the EyelinkInterface from the game instance and forwards the command.
 * The command and response are both logged via LogEyelink. Returns an error
 * message string if the game instance or device connection cannot be obtained.
 * @param command  The Eyelink command string to send.
 * @return The device response string, or an error description.
 */
FString AEyelinkGameModeBase::EyelinkSendCommand(FString command)
{
	UEyelinkGameInstance *gameInstance = Cast<UEyelinkGameInstance>(GetGameInstance());
	if (gameInstance)
	{
		UEyelinkInterface* eyelink = gameInstance->Eyelink;
		if (eyelink->IsEyelinkConnected())
		{
			eyelink->SendEyelinkCommand(command);
			FString out = eyelink->GetEyelinkResponse();
			UE_LOG(LogEyelink, Log, TEXT("Eyelink command: %s\nEyelink response: %s"), *command, *out);
		}
		return FString("Eyelink not connected");
	}
	return FString("Game Instance Not Found");
}

/**
 * Sends an integer keycode to the connected Eyelink device.
 *
 * Retrieves the EyelinkInterface from the game instance and forwards the keycode
 * as a key-press event. Logs the keycode via LogEyelink.
 * @param keycode  The integer keycode to send to the Eyelink device.
 */
void AEyelinkGameModeBase::EyelinkSendKey(int keycode)
{
	UEyelinkGameInstance *gameInstance = Cast<UEyelinkGameInstance>(GetGameInstance());
	if (gameInstance)
	{
		UEyelinkInterface* eyelink = gameInstance->Eyelink;
		if (eyelink->IsEyelinkConnected())
		{
			eyelink->SendKey(keycode);
			UE_LOG(LogEyelink, Log, TEXT("Eyelink sendking keycode %d"), keycode);
		}
	}
}