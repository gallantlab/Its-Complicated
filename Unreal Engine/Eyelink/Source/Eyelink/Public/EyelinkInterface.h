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
DECLARE_DELEGATE_RetVal(int, FSetupCalibrationDisplayDelegate);
DECLARE_DELEGATE(FExitCalibrationDisplayDelegate);

DECLARE_DELEGATE_RetVal_TwoParams(int, FInitCameraImageDisplayDelegate, int, int);
DECLARE_DELEGATE_OneParam(FUpdateCameraImageTitleDelegate, FString);
DECLARE_DELEGATE_FourParams(FDrawOneCameraImageLineDelegate, int, int, int, byte*);
DECLARE_DELEGATE(FExitCameraImageDisplayDelegate);

DECLARE_DELEGATE(FClearCalibrationDisplayDelegate);
DECLARE_DELEGATE(FEraseCalibrationTargetDelegate);
DECLARE_DELEGATE_TwoParams(FDrawCalibrationTargetDelegate, int, int);
DECLARE_DELEGATE(FGetInputDelegate);


UENUM(BlueprintType)
enum class EEyelinkInterfaceMode : uint8
{
	InitializeLibraryOnly = 0,
	Live = 1,
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
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	static bool ValidateEyelinkFileName(const FString name);

public:

	UEyelinkInterface() {};

	UEyelinkInterface(const FString eyelinkAddress);

	UEyelinkInterface(EEyelinkInterfaceMode interfaceMode);

	UEyelinkInterface(const FString eyelinkAddress, EEyelinkInterfaceMode interfaceMode);

public:
	/**
	* Initializes the interface to the eyelink *library*, not the eyetracker per se
	* This deals with library init and also setting the graphics hooks
	*/
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void InitEyelinkLibrary();

	/**
	 * Initializes communication with the eyetracker
	 * Returns true if successfully connected to eyetracker
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	bool OpenEyelinkConnection(const FString& address = FString(""), const FVector2D& resolution = FVector2D(0,0), EEyelinkInterfaceMode mode = EEyelinkInterfaceMode::Simulate);

	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void CloseEyelinkConnection();

	// == Eyelink data file commands ==

	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	bool OpenEyelinkDataFile(const FString fileName);

	UFUNCTION(BlueprintPure, Category = "eyelink")
	bool IsEyelinkDataFileOpen() const;

	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	bool CloseEyelinkDataFile(const FString fileName);

	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	bool RetrieveRemoteDataFile(const FString fileName);

	UFUNCTION(BlueprintPure, Category = "Eyelink")
	FString GetEyelinkFileName() const {return eyelinkFileName;}

	// === Wrapper methods for starting and stopping data collection ===
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void StartEyetrackingRecording();

	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void StopEyetrackingRecording();

	UFUNCTION(BlueprintPure, Category = "Eyelink")
	bool IsEyetrackingRecording() const;

	// === Eyelink interaction commands ===
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	int AcceptFixation();

	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	int SendTerminateKey();

	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	int SendBreakKey();

	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	int SendKey(int keycode);

	/**
	 * Function called to start eyetracking calibration
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void SetupCameraAndCalibrate();

	/**
	 * Function called to interrupt calibration
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void ExitCalibration() const;

	// ===  Wrapper methods for bare Eyelink C message calls ===
	// will only send if eyelink is connected and otherwise do nothing
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void SetEyelinkResolution(const FVector2D & resolution);
	   
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void ClearEyetrackingComputerDisplay();

	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void SetEyetrackingComputerStatusText(const FString message);

	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void SendTrialNumber(int trialNum);

	// == more generic functions ==
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void SendEyelinkCommand(const FString command) const;

	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void SendEyelinkMessage(const FString message) const;

	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	FString GetEyelinkResponse() const;

	UFUNCTION(BlueprintPure, Category = "Eyelink")
	bool IsEyelinkConnected() const;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Eyelink")
	FString eyelinkIP = FString("10.0.0.1");

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Eyelink")
	EEyelinkInterfaceMode InterfaceMode = EEyelinkInterfaceMode::InitializeLibraryOnly;

	// Display info
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Eyelink")
	FVector2D ScreenResolution = FVector2D();

public:
	static void UnbindDelegates();

	static FSetupCalibrationDisplayDelegate SetupCalibrationDisplayDelegate;
	static FExitCalibrationDisplayDelegate ExitCalibrationDisplayDelegate;

	static FInitCameraImageDisplayDelegate InitCameraImageDisplayDelegate;
	static FUpdateCameraImageTitleDelegate UpdateCameraImageTitleDelegate;
	static FDrawOneCameraImageLineDelegate DrawOneCameraImageLineDelegate;
	static FExitCameraImageDisplayDelegate ExitCameraImageDisplayDelegate;

	static FClearCalibrationDisplayDelegate ClearCalibrationDisplayDelegate;
	static FEraseCalibrationTargetDelegate EraseCalibrationTargetDelegate;
	static FDrawCalibrationTargetDelegate DrawCalibrationTargetDelegate;
	static FGetInputDelegate GetInputDelegate;

private:
	FString eyelinkFileName = FString("");

	//== Graphics callback function interfaces ==
	// these have to be static because the Eyelink library is all C
	// see https://www.sr-research.com/download/dispdoc/page12.html
	static INT16 SetupCalibrationDisplay();
	static void ExitCalibrationDisplay();
	static INT16 InitCameraImageDisplay(INT16, INT16);
	static void UpdateCameraImageTitle(INT16, char*);
	static void DrawOneCameraImageLine(INT16, INT16, INT16, byte*);
	static void SetCameraImagePalette(INT16, byte[], byte[], byte[]);
	static void ExitCameraImageDisplay();
	static void ClearCalibrationDisplay();
	static void EraseCalibrationTarget();
	static void DrawCalibrationTarget(INT16, INT16);

public:
	// note this actually should take a InputEvent* arg, but we use a void pointer
	// to avoid having to include the header and because we can't forward declare a union struct
	// and has to be public to enable the helper
	static INT16 GetInputKey(void*);	// TODO: figure out how to do this
	

};
