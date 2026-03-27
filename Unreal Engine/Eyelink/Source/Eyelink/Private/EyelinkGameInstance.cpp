// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "EyelinkGameInstance.h"

#include "EyelinkInterface.h"


/**
 * Constructs the game instance, creates the EyelinkInterface subobject, and
 * initializes the Eyelink C library so the device is ready before gameplay begins.
 */
UEyelinkGameInstance::UEyelinkGameInstance()
{
	// Eyelink
	// TODO: Should this happen here, or should we leave it to be called from UI at runtime?
	// I mean it seems to work so let's not worry about this TODO until somthing borks
	Eyelink = CreateDefaultSubobject<UEyelinkInterface>(TEXT("EyelinkInterface"));

	Eyelink->InitEyelinkLibrary();
}

/**
 * Gracefully shuts down the Eyelink device before the game instance is destroyed.
 * Stops any active recording, closes the open data file, and disconnects from the device.
 */
void UEyelinkGameInstance::Shutdown()
{
	if (Eyelink->IsEyelinkConnected())
	{
		if (Eyelink->IsEyetrackingRecording())
			Eyelink->StopEyetrackingRecording();
		if (Eyelink->IsEyelinkDataFileOpen())
			Eyelink->CloseEyelinkDataFile(Eyelink->GetEyelinkFileName());
		Eyelink->CloseEyelinkConnection();
	}
	Super::Shutdown();
}
