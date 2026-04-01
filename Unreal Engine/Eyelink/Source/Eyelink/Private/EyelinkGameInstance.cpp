// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "EyelinkGameInstance.h"

#include "EyelinkInterface.h"


UEyelinkGameInstance::UEyelinkGameInstance()
{
	// Eyelink
	// TODO: Should this happen here, or should we leave it to be called from UI at runtime?
	// I mean it seems to work so let's not worry about this TODO until something borks
	Eyelink = CreateDefaultSubobject<UEyelinkInterface>(TEXT("EyelinkInterface"));

	Eyelink->InitEyelinkLibrary();
}

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
