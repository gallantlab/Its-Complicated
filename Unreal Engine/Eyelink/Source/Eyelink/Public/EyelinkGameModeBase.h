// Copyright Gallant Lab, UC Berkeley

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EyelinkGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class EYELINK_API AEyelinkGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	/// Eyelink Eyetracking
	UFUNCTION(Exec, BlueprintImplementableEvent, Category = ExecFunctions, BlueprintCallable)
	void EyelinkCalibrate();

	// interrupt eyetracking
	UFUNCTION(Exec, BlueprintImplementableEvent, Category = ExecFunctions, BlueprintCallable)
	void EyelinkInterrupt();

	UFUNCTION(Exec, BlueprintImplementableEvent, Category = ExecFunctions, BlueprintCallable)
	void EyelinkValidate();
	
	UFUNCTION(Exec, Category = ExecFunctions, BlueprintCallable)
	FString EyelinkSendCommand(FString command);

	UFUNCTION(Exec, Category = ExecFunctions, BlueprintCallable)
	void EyelinkSendKey(int keycode);
};
