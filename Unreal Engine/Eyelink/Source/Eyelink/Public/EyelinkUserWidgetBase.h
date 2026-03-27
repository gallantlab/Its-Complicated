

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EyelinkUserWidgetBase.generated.h"

class UEyelinkInterface;
/**
 * A base UMG widget class for binding Eyelink graphics callbacks to UI elements.
 *
 * Subclass this widget in UMG to implement the visual side of Eyelink calibration.
 * The calibration-related Blueprint-implementable events are invoked automatically
 * once the delegates are bound via BindEyelinkDelegates.
 */
UCLASS()
class EYELINK_API UEyelinkUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()
public:
	/**
	 * Constructs the widget with the given object initializer.
	 * @param ObjectInitializer  Unreal object initializer forwarded to the parent class.
	 */
	UEyelinkUserWidgetBase(const FObjectInitializer& ObjectInitializer);

	/**
	 * Binds this widget's drawing functions to the provided Eyelink interface object.
	 *
	 * After calling this, the Eyelink library's calls to draw things will be bound to
	 * the blueprint events implemented in this widget.
	 * @param eyelink  The EyelinkInterface to bind delegates on.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void BindEyelinkDelegates(UEyelinkInterface* eyelink);

	/**
	 * Unbinds all delegates from the Eyelink interface object.
	 * Call this when the calibration session has ended.
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void UnbindEyelinkDelegates();

	// === Functions to bind to the Eyelink calibration delegates ===

	/**
	 * Blueprint event called by Eyelink to initialize the calibration display.
	 * Implement in Blueprint to show the calibration UI.
	 * @return 0 on success.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Eyelink", meta = (ForceAsFunction))
	int SetupCalibrationDisplay();

	/**
	 * Blueprint event called by Eyelink to tear down the calibration display.
	 * Implement in Blueprint to hide the calibration UI.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Eyelink", meta = (ForceAsFunction))
	void ExitCalibrationDisplay();

	/**
	 * Blueprint event called by Eyelink to clear all content from the calibration display.
	 * Implement in Blueprint to blank the calibration UI without fully hiding it.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Eyelink", meta = (ForceAsFunction))
	void ClearCalibrationDisplay();

	/**
	 * Blueprint event called by Eyelink to erase the current calibration target.
	 * Implement in Blueprint to hide or remove the target indicator.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Eyelink", meta = (ForceAsFunction))
	void HideCalibrationTarget();

	/**
	 * Blueprint event called by Eyelink to draw a calibration target at the given position.
	 * Implement in Blueprint to render a fixation dot or crosshair at (x, y).
	 * @param x  Horizontal screen position in pixels.
	 * @param y  Vertical screen position in pixels.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Eyelink", meta = (ForceAsFunction))
	void DrawCalibrationTarget(int x, int y);

	/**
	 * Returns the EyelinkInterface associated with the current game instance.
	 * @return Pointer to the UEyelinkInterface, or nullptr if unavailable.
	 */
	UFUNCTION(BlueprintPure, Category = "Eyelink")
	UEyelinkInterface* GetEyelink() const;

	/**
	 * Returns the reciprocal of the current DPI scale factor for this viewport.
	 *
	 * Useful when converting between logical widget coordinates and physical screen pixels
	 * (e.g. for positioning calibration targets correctly on high-DPI displays).
	 * @return 1/DPIScale, or 1.0 if no owning player or viewport is available.
	 */
	UFUNCTION(BlueprintPure, Category = "Eyelink")
	float GetDPIScaleFactor() const;

private:
	/** Reference to the Eyelink interface object whose delegates are bound to this widget. */
	UEyelinkInterface* eyelink;	// reference to eyelink object	
};
