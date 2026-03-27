#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EyelinkInterface.generated.h"

// NOTE: do not include the Eyelink headers in Unreal headers because
// Eyelink uses winsocks 1 while Unreal uses winsocks2, and that gives
// A bunch of compilation errors.

// forward def eyelink types
typedef short INT16;
typedef unsigned char byte;

// Delegate declarations

/** Delegate for Eyelink to set up the calibration display. Returns 0 on success. */
DECLARE_DELEGATE_RetVal(int, FSetupCalibrationDisplayDelegate);
/** Delegate for Eyelink when the calibration display should be torn down. */
DECLARE_DELEGATE(FExitCalibrationDisplayDelegate);

/** Delegate for initializing the camera image display with the given width and height. Returns 0 on success. */
DECLARE_DELEGATE_RetVal_TwoParams(int, FInitCameraImageDisplayDelegate, int, int);
/** Delegate for updating the camera image title string. */
DECLARE_DELEGATE_OneParam(FUpdateCameraImageTitleDelegate, FString);
/** Delegate for drawing a single horizontal line of camera image data. Args: width, line index, total lines, pixel data. */
DECLARE_DELEGATE_FourParams(FDrawOneCameraImageLineDelegate, int, int, int, byte*);
/** Delegate for exiting the camera image display. */
DECLARE_DELEGATE(FExitCameraImageDisplayDelegate);

/** Delegate for clearing the entire calibration display. */
DECLARE_DELEGATE(FClearCalibrationDisplayDelegate);
/** Delegate for erasing the current calibration target from the display. */
DECLARE_DELEGATE(FEraseCalibrationTargetDelegate);
/** Delegate for drawing a calibration target at the given (x, y) screen coordinates. */
DECLARE_DELEGATE_TwoParams(FDrawCalibrationTargetDelegate, int, int);
/** Delegate for polling for user input during calibration. */
DECLARE_DELEGATE(FGetInputDelegate);


/**
 * Operating modes for the Eyelink library.
 */
UENUM(BlueprintType)
enum class EEyelinkInterfaceMode : uint8
{
	/** Only initialize the Eyelink C library; do not open connection. */
	InitializeLibraryOnly = 0,
	/** Connect to a real Eyelink host over the network. */
	Live = 1,
	/** Run in simulation mode (no connection to real Eyelink host). */
	Simulate = 2,
};


/**
 * A class to bridge the Eyelink C API with Unreal C++
 *
 * Long story short: The Eyelink library makes strong assumptions about the linearity
 * of experiments, which cannot be satisfied in a complex OOP environment. Here we
 * encapsulate information about the Eyelink machine in a single object. Because the
 * Eyelink machine will make incoming calls to display calibration targets, we encapsulate
 * these calls in delegates so that they only will be handled when needed.
 *
 * Because Eyelink is in C, these delegates have to be static. Thus the class should be
 * used as a singleton. The delegates should only be bound minimally, i.e. bind them right
 * before we do calibration, and unbind right after. This serves two purposes. First,
 * we minimize how much effect the Eyelink machine can have on gameplay. Second, Eyelink,
 * being C, has no concept of object lifecycles; we use UMG to display eyetracking and
 * those are ephemeral.
 *
 * The delegate binding should be done externally. I.e. this should be a reactive-only object.
 * Eyelink, in no circumstances, should be able to affect gameplay unless in explicit response
 * to a function call to this object.
 *
 * Lifecycles-wise, because Eyelink is C, it does not seem like the library is elegantly
 * capable of handling start-stops / multiple instances. So this object should be used
 * by GameInstance, which will keep everything alive for the duration of the experiment.
 */
UCLASS(BlueprintType)
class EYELINK_API UEyelinkInterface : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Validates that a filename conforms to Eyelink EDF naming rules.
	 *
	 * EDF filenames must start with an alphanumeric character, contain only
	 * alphanumeric characters or underscores, be at most 8 characters before
	 * the extension, and end with ".edf".
	 * @param name  The candidate filename to validate.
	 * @return true if the filename is valid for use with the Eyelink host.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	static bool ValidateEyelinkFileName(const FString name);

public:

	/** Default constructor. Creates an interface with default settings. */
	UEyelinkInterface() {};

	/**
	 * Constructs the interface with a specific Eyelink host IP address.
	 * @param eyelinkAddress  The IP address of the Eyelink host computer.
	 */
	UEyelinkInterface(const FString eyelinkAddress);

	/**
	 * Constructs the interface with a specific operating mode.
	 * @param interfaceMode  The mode in which the interface should operate.
	 */
	UEyelinkInterface(EEyelinkInterfaceMode interfaceMode);

	/**
	 * Constructs the interface with both a host IP address and an operating mode.
	 * @param eyelinkAddress  The IP address of the Eyelink host computer.
	 * @param interfaceMode   The mode in which the interface should operate.
	 */
	UEyelinkInterface(const FString eyelinkAddress, EEyelinkInterfaceMode interfaceMode);

public:
	/**
	 * Initializes the Eyelink C library and registers all graphics callback hooks.
	 *
	 * Must be called once before any other Eyelink operations.
	 * This deals with library initialization and setting up the display/input hooks
	 * that Eyelink uses to drive calibration UI.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void InitEyelinkLibrary();

	/**
	 * Opens a network connection to the Eyelink eyetracker.
	 *
	 * Configures the device address, screen resolution, and operating mode, then
	 * attempts to establish a connection. On success the tracker is placed in
	 * offline (non-realtime) mode and sets what should be recorded (lifted from stimulus_presentation repo).
	 * @param address     IP address of the Eyelink host computer. Uses the stored eyelinkIP if empty.
	 * @param resolution  Display resolution in pixels. Uses the stored ScreenResolution if zero.
	 * @param mode        Operating mode (Live or Simulate).
	 * @return true if the connection was established successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	bool OpenEyelinkConnection(const FString& address = FString(""), const FVector2D& resolution = FVector2D(0,0), EEyelinkInterfaceMode mode = EEyelinkInterfaceMode::Simulate);

	/**
	 * Closes the connection to the Eyelink host if one is currently open.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void CloseEyelinkConnection();

	// == Eyelink data file commands ==

	/**
	 * Opens a new EDF data file on the Eyelink host computer for recording.
	 * @param fileName  Name of the EDF file to create. Must satisfy ValidateEyelinkFileName.
	 * @return true if the data file was opened successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	bool OpenEyelinkDataFile(const FString fileName);

	/**
	 * Returns whether an Eyelink data file is currently open.
	 * @return true if a data file has been opened and not yet closed.
	 */
	UFUNCTION(BlueprintPure, Category = "eyelink")
	bool IsEyelinkDataFileOpen() const;

	/**
	 * Closes the currently open EDF data file on the Eyelink host computer.
	 * Eyelink will close the current file regardless of the name here.
	 * @param fileName  Name of the EDF file to close (for logging/verification purposes).
	 * @return true if the file was closed successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	bool CloseEyelinkDataFile(const FString fileName);

	/**
	 * Transfers the EDF data file from the Eyelink host computer to the local machine.
	 * The file is saved into the project's Saved directory.
	 * @param fileName  Name of the EDF file to retrieve.
	 * @return true if the file was transferred successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	bool RetrieveRemoteDataFile(const FString fileName);

	/**
	 * Returns the name of the currently open (or most recently opened) EDF data file.
	 * @return The EDF filename string, or an empty string if no file is open.
	 */
	UFUNCTION(BlueprintPure, Category = "Eyelink")
	FString GetEyelinkFileName() const {return eyelinkFileName;}

	// === Wrapper methods for starting and stopping data collection ===

	/**
	 * Starts eyetracking data recording on the Eyelink host.
	 * Does nothing if the device is not connected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void StartEyetrackingRecording();

	/**
	 * Stops eyetracking data recording on the Eyelink host.
	 * Does nothing if the device is not connected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void StopEyetrackingRecording();

	/**
	 * Returns whether the Eyelink host is currently recording eye data.
	 * @return true if recording is active.
	 */
	UFUNCTION(BlueprintPure, Category = "Eyelink")
	bool IsEyetrackingRecording() const;

	// === Eyelink interaction commands ===

	/**
	 * Tells the host to accept the current fixation for the current calibration point.
	 * @return The Eyelink library return value; -1 if not connected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	int AcceptFixation();

	/**
	 * Sends the Eyelink "terminate" key to end a trial.
	 * @return The Eyelink library return value; -1 if not connected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	int SendTerminateKey();

	/**
	 * Sends the Eyelink "break" key to abort processing.
	 * @return The Eyelink library return value; -1 if not connected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	int SendBreakKey();

	/**
	 * Sends an arbitrary keycode to the Eyelink host as a key-press event.
	 * @param keycode  The integer keycode to send.
	 * @return The Eyelink library return value; 0 if not connected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	int SendKey(int keycode);

	/**
	 * Start the Eyelink camera setup and calibration routine.
	 * Does nothing if the device is not connected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void SetupCameraAndCalibrate();

	/**
	 * Interrupts the current calibration or drift-correction routine.
	 * Does nothing if the device is not connected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void ExitCalibration() const;

	// ===  Wrapper methods for bare Eyelink C message calls ===
	// will only send if eyelink is connected and otherwise do nothing

	/**
	 * Sends the display resolution to the Eyelink host computer so it can
	 * correctly map gaze coordinates to screen pixels.
	 * @param resolution  Display dimensions in pixels (width x height).
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void SetEyelinkResolution(const FVector2D & resolution);

	/**
	 * Sends a command to clear the Eyelink host's display.
	 * Does nothing if the device is not connected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void ClearEyetrackingComputerDisplay();

	/**
	 * Sets the status text shown on the Eyelink host's display.
	 * @param message  The message string to display on the Eyelink host.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void SetEyetrackingComputerStatusText(const FString message);

	/**
	 * Sends the current trial number as a message to the Eyelink data file.
	 * @param trialNum  The trial number to record.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void SendTrialNumber(int trialNum);

	// == more generic functions ==

	/**
	 * Sends a raw command string to the Eyelink host computer.
	 * Does nothing if the device is not connected.
	 * @param command  The command string to send (Eyelink command syntax).
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void SendEyelinkCommand(const FString command) const;

	/**
	 * Writes a timestamped message into the active EDF data file.
	 * Does nothing if the device is not connected.
	 * @param message  The message string to embed in the data file.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void SendEyelinkMessage(const FString message) const;

	/**
	 * Retrieves the most recent response string from the Eyelink host.
	 * @return The response string, or an empty string if not connected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	FString GetEyelinkResponse() const;

	/**
	 * Returns whether the Eyelink host is currently connected.
	 * @return true if an active connection exists.
	 */
	UFUNCTION(BlueprintPure, Category = "Eyelink")
	bool IsEyelinkConnected() const;

	/** IP address of the Eyelink host computer. Defaults to "10.0.0.1". */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Eyelink")
	FString eyelinkIP = FString("10.0.0.1");

	/** Current operating mode of the interface (library-only, live, or simulate). */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Eyelink")
	EEyelinkInterfaceMode InterfaceMode = EEyelinkInterfaceMode::InitializeLibraryOnly;

	/** Screen resolution reported to the Eyelink host for gaze coordinate mapping. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Eyelink")
	FVector2D ScreenResolution = FVector2D();

public:
	/**
	 * Unbinds all currently bound calibration and camera-image delegates.
	 * Call this after calibration is complete to minimize Eyelink's influence on gameplay.
	 */
	static void UnbindDelegates();

	/** Delegate fired by Eyelink to request calibration display setup. */
	static FSetupCalibrationDisplayDelegate SetupCalibrationDisplayDelegate;
	/** Delegate fired by Eyelink to tear down the calibration display. */
	static FExitCalibrationDisplayDelegate ExitCalibrationDisplayDelegate;

	/** Delegate fired by Eyelink to initialize the live camera image display. */
	static FInitCameraImageDisplayDelegate InitCameraImageDisplayDelegate;
	/** Delegate fired by Eyelink to update the camera image window title. */
	static FUpdateCameraImageTitleDelegate UpdateCameraImageTitleDelegate;
	/** Delegate fired by Eyelink to supply one horizontal scan line of camera pixel data. */
	static FDrawOneCameraImageLineDelegate DrawOneCameraImageLineDelegate;
	/** Delegate fired by Eyelink to close the camera image display. */
	static FExitCameraImageDisplayDelegate ExitCameraImageDisplayDelegate;

	/** Delegate fired by Eyelink to clear the calibration display. */
	static FClearCalibrationDisplayDelegate ClearCalibrationDisplayDelegate;
	/** Delegate fired by Eyelink to remove the current calibration target. */
	static FEraseCalibrationTargetDelegate EraseCalibrationTargetDelegate;
	/** Delegate fired by Eyelink to draw a calibration target at screen coordinates (x, y). */
	static FDrawCalibrationTargetDelegate DrawCalibrationTargetDelegate;
	/** Delegate fired by Eyelink to poll for keyboard input during calibration. */
	static FGetInputDelegate GetInputDelegate;

private:
	/** Filename of the currently open (or most recently used) EDF data file. */
	FString eyelinkFileName = FString("");

	//== Graphics callback function interfaces ==
	// these have to be static because the Eyelink library is all C
	// see https://www.sr-research.com/download/dispdoc/page12.html

	/** Static callback: called by Eyelink to set up the calibration display. */
	static INT16 SetupCalibrationDisplay();
	/** Static callback: called by Eyelink to tear down the calibration display. */
	static void ExitCalibrationDisplay();
	/** Static callback: called by Eyelink to initialize the camera image display. */
	static INT16 InitCameraImageDisplay(INT16, INT16);
	/** Static callback: called by Eyelink to update the camera image title. */
	static void UpdateCameraImageTitle(INT16, char*);
	/** Static callback: called by Eyelink to provide one line of camera image pixel data. */
	static void DrawOneCameraImageLine(INT16, INT16, INT16, byte*);
	/** Static callback: called by Eyelink to set the color palette for the camera image. */
	static void SetCameraImagePalette(INT16, byte[], byte[], byte[]);
	/** Static callback: called by Eyelink to close the camera image display. */
	static void ExitCameraImageDisplay();
	/** Static callback: called by Eyelink to clear the calibration display. */
	static void ClearCalibrationDisplay();
	/** Static callback: called by Eyelink to erase the current calibration target. */
	static void EraseCalibrationTarget();
	/** Static callback: called by Eyelink to draw a calibration target at (x, y). */
	static void DrawCalibrationTarget(INT16, INT16);

public:
	// note this actually should take a InputEvent* arg, but we use a void pointer
	// to avoid having to include the header and because we can't forward declare a union struct
	// and has to be public to enable the helper
	/**
	 * Static callback: called by Eyelink to retrieve the next input key event.
	 *
	 * Uses a void pointer instead of InputEvent* to avoid including the Eyelink
	 * header (which causes winsock conflicts) and because a union struct cannot
	 * be forward-declared.
	 * @param event  Opaque pointer to the InputEvent provided by the Eyelink library.
	 * @return The key code to return to Eyelink (currently always 0).
	 */
	static INT16 GetInputKey(void*);	// TODO: figure out how to do this
	

};
