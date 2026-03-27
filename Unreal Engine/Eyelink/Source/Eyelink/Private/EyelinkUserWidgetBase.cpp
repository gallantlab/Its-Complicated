

#include "EyelinkUserWidgetBase.h"
#include "EyelinkInterface.h"
#include "EyelinkGameInstance.h"
#include "Engine/UserInterfaceSettings.h"


/** Calls the parent UUserWidget constructor with the given object initializer. */
UEyelinkUserWidgetBase::UEyelinkUserWidgetBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

/**
 * Binds this widget's drawing functions to the provided Eyelink interface object.
 *
 * Stores a reference to the interface and binds all calibration display delegates
 * (setup, exit, clear, erase target, draw target) to the corresponding Blueprint
 * implementable events on this widget.
 */
void UEyelinkUserWidgetBase::BindEyelinkDelegates(UEyelinkInterface* eyelink)
{
	this->eyelink = eyelink;

	eyelink->SetupCalibrationDisplayDelegate.BindUObject(this, &UEyelinkUserWidgetBase::SetupCalibrationDisplay);
	eyelink->ExitCalibrationDisplayDelegate.BindUObject(this, &UEyelinkUserWidgetBase::ExitCalibrationDisplay);
	eyelink->ClearCalibrationDisplayDelegate.BindUObject(this, &UEyelinkUserWidgetBase::ClearCalibrationDisplay);
	eyelink->EraseCalibrationTargetDelegate.BindUObject(this, &UEyelinkUserWidgetBase::HideCalibrationTarget);
	eyelink->DrawCalibrationTargetDelegate.BindUObject(this, &UEyelinkUserWidgetBase::DrawCalibrationTarget);
}

/**
 * Unbinds all delegates from the stored Eyelink interface object.
 * Safe to call even if no delegates are currently bound.
 */
void UEyelinkUserWidgetBase::UnbindEyelinkDelegates()
{
	if (eyelink)
		eyelink->UnbindDelegates();
}

/**
 * Retrieves the EyelinkInterface from the current game instance.
 * @return Pointer to UEyelinkInterface, or nullptr if the game instance is not an UEyelinkGameInstance.
 */
UEyelinkInterface* UEyelinkUserWidgetBase::GetEyelink() const
{
	UEyelinkGameInstance* gameInstance = Cast<UEyelinkGameInstance>(GetWorld()->GetGameInstance());
	if (gameInstance)
		return gameInstance->Eyelink;
	return nullptr;
}

/**
 * Calculates the inverse of the DPI scale factor for the current viewport.
 *
 * Queries the owning player's viewport size and looks up the DPI scale using
 * UUserInterfaceSettings. Returns 1/DPIScale so callers can convert from
 * logical widget coordinates to physical screen pixels.
 * @return 1 / DPIScaleFactor, or 1.0 if no owning player or controller is available.
 */
float UEyelinkUserWidgetBase::GetDPIScaleFactor() const
{
	int width, height;
	APlayerController* controller = GetOwningLocalPlayer() == nullptr ? nullptr : GetOwningLocalPlayer()->GetPlayerController(GetWorld());
	if (controller)
	{
		controller->GetViewportSize(width, height);
		return 1.0 / GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())->GetDPIScaleBasedOnSize(FIntPoint(width, height));
	}
	return 1;

}