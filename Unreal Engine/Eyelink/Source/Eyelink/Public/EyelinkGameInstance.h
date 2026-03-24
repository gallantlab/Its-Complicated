// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EyelinkGameInstance.generated.h"

class UEyelinkInterface;
/**
 * A game instance that contains an eyelink object
 * Eyelink lives here because the GameInstance lifecycle spans the entire
 * time the game is running, and Eyelink is a C library and doesn't have
 * object concepts. So if it lives here, the eyelink C library gets initialized
 * once per game.
 */
UCLASS()
class EYELINK_API UEyelinkGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UEyelinkGameInstance();

	virtual void Shutdown() override;
	
	//============================================
	// Eyelink
	//============================================
public:
	UPROPERTY(Category = "Eyelink", BlueprintReadOnly)
	UEyelinkInterface* Eyelink;
	
	
};
