
#include "EyelinkInterface.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Async/Async.h"
#include <eyelink.h>
#include <core_expt.h>

#include "Regex.h"
#include "Eyelink/Eyelink.h"

// Static delegate definitions
FSetupCalibrationDisplayDelegate UEyelinkInterface::SetupCalibrationDisplayDelegate;
FExitCalibrationDisplayDelegate UEyelinkInterface::ExitCalibrationDisplayDelegate;

FInitCameraImageDisplayDelegate UEyelinkInterface::InitCameraImageDisplayDelegate;
FUpdateCameraImageTitleDelegate UEyelinkInterface::UpdateCameraImageTitleDelegate;
FDrawOneCameraImageLineDelegate UEyelinkInterface::DrawOneCameraImageLineDelegate;
FExitCameraImageDisplayDelegate UEyelinkInterface::ExitCameraImageDisplayDelegate;

FClearCalibrationDisplayDelegate UEyelinkInterface::ClearCalibrationDisplayDelegate;
FEraseCalibrationTargetDelegate UEyelinkInterface::EraseCalibrationTargetDelegate;
FDrawCalibrationTargetDelegate UEyelinkInterface::DrawCalibrationTargetDelegate;
FGetInputDelegate UEyelinkInterface::GetInputDelegate;

// Note for eyelink functions that have a bool-int return value
// it's better to do an explicit compare-to-0 than to act as
// if they return a bool

bool UEyelinkInterface::ValidateEyelinkFileName(const FString name)
{
	static const FRegexPattern EyelinkNameFormat("^[A-Za-z0-9][A-Za-z0-9_]{0,7}.edf$");
	FRegexMatcher matcher(EyelinkNameFormat, name);
	return matcher.FindNext();
}

UEyelinkInterface::UEyelinkInterface(const FString eyelinkAddress)
{
	eyelinkIP = eyelinkAddress;
}

UEyelinkInterface::UEyelinkInterface(EEyelinkInterfaceMode interfaceMode)
{
	InterfaceMode = interfaceMode;
}

UEyelinkInterface::UEyelinkInterface(const FString eyelinkAddress, EEyelinkInterfaceMode interfaceMode)
{
	eyelinkIP = eyelinkAddress;
	InterfaceMode = interfaceMode;
}

INT16 UEyelinkInterface::GetInputKey(void*)
{
	return 0;
}

static INT16 InputWrap(InputEvent* event)
{
	return UEyelinkInterface::GetInputKey((void*)event);
}

/**
 * @brief
 */
void UEyelinkInterface::InitEyelinkLibrary()
{
	HOOKFCNS eyelinkCallbackFunctions;
	memset(&eyelinkCallbackFunctions, 0, sizeof(eyelinkCallbackFunctions)); // because C

	// set function pointers for Eyelink callback graphics
	eyelinkCallbackFunctions.setup_cal_display_hook = &SetupCalibrationDisplay;
	eyelinkCallbackFunctions.exit_cal_display_hook = &ExitCalibrationDisplay;
	eyelinkCallbackFunctions.setup_image_display_hook = &InitCameraImageDisplay;
	eyelinkCallbackFunctions.image_title_hook = &UpdateCameraImageTitle;
	eyelinkCallbackFunctions.draw_image_line_hook = &DrawOneCameraImageLine;
	eyelinkCallbackFunctions.set_image_palette_hook = &SetCameraImagePalette;
	eyelinkCallbackFunctions.exit_image_display_hook = &ExitCameraImageDisplay;
	eyelinkCallbackFunctions.clear_cal_display_hook = &ClearCalibrationDisplay;
	eyelinkCallbackFunctions.erase_cal_target_hook = &EraseCalibrationTarget;
	eyelinkCallbackFunctions.draw_cal_target_hook = &DrawCalibrationTarget;
	eyelinkCallbackFunctions.cal_target_beep_hook = nullptr;
	eyelinkCallbackFunctions.cal_done_beep_hook = nullptr;
	eyelinkCallbackFunctions.dc_done_beep_hook = nullptr;
	eyelinkCallbackFunctions.dc_target_beep_hook = nullptr;
	eyelinkCallbackFunctions.get_input_key_hook = &InputWrap;

	// provides callback functions for the Eyelink core library to be able to draw stuff
	setup_graphic_hook_functions(&eyelinkCallbackFunctions);

	// TODO: other library init things
	// maybe if there's a way to disable eyelink's input scanning
	// sets special keys to be some extended ASCII that doesn't reflect real keys
	// special keys are the ESC and CTRL-C things
	eyelink_set_special_keys(0x0000, 0x0090, 0x0000, 0x00A0, 0);
}


bool UEyelinkInterface::OpenEyelinkConnection(const FString& address, const FVector2D& resolution, EEyelinkInterfaceMode mode)
{
	// set properties if given
	if (!address.Equals(FString("")))
		eyelinkIP = address;
	if (!resolution.Equals(FVector2D(0, 0)))
		ScreenResolution = resolution;
	InterfaceMode = mode;

	// set eyelink IP
	set_eyelink_address(TCHAR_TO_ANSI(*eyelinkIP));

	// TODO: this
	// open the connection
	if (open_eyelink_connection((INT16)InterfaceMode - 1) != 0)
		return false;

	// !! Important !!
	// never run the eyelink library in realtime mode
	set_offline_mode();

	// can we live without the shadow getkey system

	// tell the eyetracker about our display information
	SetEyelinkResolution(resolution);

	// TODO: eyelink version stuff. Here we are hardcoding for V3

	// set  what data is to be saved in files and passed in real time
	SendEyelinkCommand(FString("file_event_filter = LEFT,RIGHT,FIXATION,SACCADE,BLINK,MESSAGE,BUTTON"));
	SendEyelinkCommand(FString("file_sample_data = LEFT,RIGHT,GAZE,AREA,GAZERES,STATUS,INPUT"));

	return true;

}

void UEyelinkInterface::CloseEyelinkConnection()
{
	if (IsEyelinkConnected())
	{
		close_eyelink_connection();
	}
}

bool UEyelinkInterface::OpenEyelinkDataFile(const FString fileName)
{
	if (IsEyelinkConnected())
	{
		// tell the eyetracker to create a new savefile
		int retval = open_data_file(TCHAR_TO_ANSI(*fileName));
		if (retval != 0)
			return false;
		// write a message into the savefile

		eyelinkFileName = fileName;

		return true;
	}
	return false;
}


bool UEyelinkInterface::CloseEyelinkDataFile(const FString fileName)
{
	if (IsEyelinkConnected())
	{
		return close_data_file() == 0;
	}
	return true;
}


bool UEyelinkInterface::IsEyelinkDataFileOpen() const
{
	return !eyelinkFileName.Equals(FString(""));
}

void UEyelinkInterface::SetupCameraAndCalibrate()
{
	if (IsEyelinkConnected())
	{
		AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, []()
		{
			do_tracker_setup();
		});
	}
}

void UEyelinkInterface::ExitCalibration() const
{
	if (IsEyelinkConnected())
	{
		exit_calibration();
	}
}

void UEyelinkInterface::StartEyetrackingRecording()
{
	if (IsEyelinkConnected())
	{
		start_recording(1, 1, 0, 0);
	}
}

void UEyelinkInterface::StopEyetrackingRecording()
{
	if (IsEyelinkConnected())
	{
		stop_recording();
	}
}

bool UEyelinkInterface::IsEyetrackingRecording() const
{
	if (IsEyelinkConnected())
	{
		return check_recording() == 0;
	}
	return false;
}

int UEyelinkInterface::AcceptFixation()
{
	if (IsEyelinkConnected())
	{
		return eyelink_accept_trigger();
	}
	return -1;
}

int UEyelinkInterface::SendTerminateKey()
{
	if (IsEyelinkConnected())
	{
		return eyelink_send_keybutton(0x0090, 0x0000, KB_PRESS);
	}
	return -1;
}


void UEyelinkInterface::SetEyetrackingComputerStatusText(const FString message)
{
	// TODO: this
}

void UEyelinkInterface::SendTrialNumber(int trialNum)
{
	// TODO: this
}



void UEyelinkInterface::ClearEyetrackingComputerDisplay()
{
	if (IsEyelinkConnected())
	{
		eyecmd_printf("clear_screen 0");
	}
}

void UEyelinkInterface::SendEyelinkCommand(const FString command) const
{
	if (IsEyelinkConnected())
	{
		eyecmd_printf(TCHAR_TO_ANSI(*command));
	}
}

void UEyelinkInterface::SendEyelinkMessage(const FString message) const
{
	if (IsEyelinkConnected())
	{
		eyemsg_printf(TCHAR_TO_ANSI(*message));
	}
}

FString UEyelinkInterface::GetEyelinkResponse() const
{
	if (IsEyelinkConnected())
	{
		int res = eyelink_command_result();
		char FARTYPE message[256];
		int length = eyelink_last_message(message);
		message[length] = '\0';	// Just to make sure we temrinate the string correctly
		return FString(message);
	}
	return FString("");
}

int UEyelinkInterface::SendBreakKey()
{
	if (IsEyelinkConnected())
	{
		return eyelink_send_keybutton(0x00A0, 0x0000, KB_PRESS);
	}
	return -1;
}

int UEyelinkInterface::SendKey(int keycode)
{
	if (IsEyelinkConnected())
	{
		return eyelink_send_keybutton((UINT16)keycode, 0, KB_PRESS);
	}
	return 0;
}

void UEyelinkInterface::SetEyelinkResolution(const FVector2D& resolution)
{
	if (IsEyelinkConnected())
	{
		int width = (int)resolution.X;
		int height = (int)resolution.Y;
		eyecmd_printf("screen_pixel_coords = 0 0 %ld %ld", width - 1, height - 1);
		eyemsg_printf("DISPLAY_COORDS 0 0 %ld %ld", width - 1, height - 1);
	}
}


void UEyelinkInterface::UnbindDelegates()
{
	if (ExitCalibrationDisplayDelegate.IsBound())
		UEyelinkInterface::ExitCalibrationDisplayDelegate.Unbind();
	if (SetupCalibrationDisplayDelegate.IsBound())
		UEyelinkInterface::SetupCalibrationDisplayDelegate.Unbind();
	if (InitCameraImageDisplayDelegate.IsBound())
		UEyelinkInterface::InitCameraImageDisplayDelegate.Unbind();
	if (UpdateCameraImageTitleDelegate.IsBound())
		UEyelinkInterface::UpdateCameraImageTitleDelegate.Unbind();
	if (DrawOneCameraImageLineDelegate.IsBound())
		UEyelinkInterface::DrawOneCameraImageLineDelegate.Unbind();
	if (ExitCameraImageDisplayDelegate.IsBound())
		UEyelinkInterface::ExitCameraImageDisplayDelegate.Unbind();
	if (ClearCalibrationDisplayDelegate.IsBound())
		UEyelinkInterface::ClearCalibrationDisplayDelegate.Unbind();
	if (EraseCalibrationTargetDelegate.IsBound())
		UEyelinkInterface::EraseCalibrationTargetDelegate.Unbind();
	if (DrawCalibrationTargetDelegate.IsBound())
		UEyelinkInterface::DrawCalibrationTargetDelegate.Unbind();
	if (GetInputDelegate.IsBound())
		UEyelinkInterface::GetInputDelegate.Unbind();
}



// === Delegate calls for the eyelink ===
INT16 UEyelinkInterface::SetupCalibrationDisplay()
{
	if (UEyelinkInterface::SetupCalibrationDisplayDelegate.IsBound())
	{
		UE_LOG(LogEyelink, Log, TEXT("Set up calibration display delegate called"));
		AsyncTask(ENamedThreads::GameThread, []()
		{
			UEyelinkInterface::SetupCalibrationDisplayDelegate.Execute(); // TODO: this line generates some sort of UE4 message log exception
		});
	}
	return 0;
}

void UEyelinkInterface::ExitCalibrationDisplay()
{
	UE_LOG(LogEyelink, Log, TEXT("Exit calibration display delegate called"));
	UEyelinkInterface::ExitCalibrationDisplayDelegate.ExecuteIfBound();
}

INT16 UEyelinkInterface::InitCameraImageDisplay(INT16 width, INT16 height)
{
	if (UEyelinkInterface::InitCameraImageDisplayDelegate.IsBound())
	{
		UE_LOG(LogEyelink, Log, TEXT("Init camera image display delegate called width %d height %d"), width, height);
		AsyncTask(ENamedThreads::GameThread, [width, height]()
		{
			UEyelinkInterface::InitCameraImageDisplayDelegate.Execute(width, height);
		});
	}
	return 0;
}

void UEyelinkInterface::UpdateCameraImageTitle(INT16 threshold, char* title)
{
	UE_LOG(LogEyelink, Log, TEXT("Update camera image title delegate called threshold %d title %s"), threshold, ANSI_TO_TCHAR(title));
	AsyncTask(ENamedThreads::GameThread, [title]()
	{
		UEyelinkInterface::UpdateCameraImageTitleDelegate.ExecuteIfBound(ANSI_TO_TCHAR(title));
	});
}

void UEyelinkInterface::DrawOneCameraImageLine(INT16 width, INT16 line, INT16 numLines, byte* pixels)
{
	UE_LOG(LogEyelink, Log, TEXT("Draw one camera image line delegate called width %d line %d numLines %d"), width, line, numLines);
	UEyelinkInterface::DrawOneCameraImageLineDelegate.ExecuteIfBound(width, line, numLines, pixels);
}

void UEyelinkInterface::SetCameraImagePalette(INT16, byte[], byte[], byte[])
{
	UE_LOG(LogEyelink, Log, TEXT("Set camera image palette delegate called"));
	//TODO: this
}

void UEyelinkInterface::ExitCameraImageDisplay()
{
	UE_LOG(LogEyelink, Log, TEXT("Exit camera image display delegate called"));
	UEyelinkInterface::ExitCameraImageDisplayDelegate.ExecuteIfBound();
}

void UEyelinkInterface::ClearCalibrationDisplay()
{
	UE_LOG(LogEyelink, Log, TEXT("Clear calibration diisplay delegate called"));
	AsyncTask(ENamedThreads::GameThread, []()
	{
		UEyelinkInterface::ClearCalibrationDisplayDelegate.ExecuteIfBound();
	});
}

void UEyelinkInterface::EraseCalibrationTarget()
{
	UE_LOG(LogEyelink, Log, TEXT("Erase calibration target delegate called"));
	AsyncTask(ENamedThreads::GameThread, []()
	{
		UEyelinkInterface::EraseCalibrationTargetDelegate.ExecuteIfBound();
	});
}

void UEyelinkInterface::DrawCalibrationTarget(INT16 x, INT16 y)
{
	UE_LOG(LogEyelink, Log, TEXT("Draw calibration target delegate called x %d y %d"), x, y);
	AsyncTask(ENamedThreads::GameThread, [x, y]()
	{
		UEyelinkInterface::DrawCalibrationTargetDelegate.ExecuteIfBound(x, y);
	});
}

bool UEyelinkInterface::IsEyelinkConnected() const
{
	return eyelink_is_connected();
}

bool UEyelinkInterface::RetrieveRemoteDataFile(const FString fileName)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString localFileName = PlatformFile.ConvertToAbsolutePathForExternalAppForWrite(*FPaths::Combine(FPaths::ProjectSavedDir(), fileName));
	int retval = receive_data_file(TCHAR_TO_ANSI(*fileName), TCHAR_TO_ANSI(*localFileName), 0);
	return retval > 0;
}