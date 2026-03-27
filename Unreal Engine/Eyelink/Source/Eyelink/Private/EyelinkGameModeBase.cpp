// Copyright Gallant Lab, UC Berkeley

#include "EyelinkGameModeBase.h"

#include "Eyelink.h"
#include "EyelinkGameInstance.h"
#include "EyelinkInterface.h"


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