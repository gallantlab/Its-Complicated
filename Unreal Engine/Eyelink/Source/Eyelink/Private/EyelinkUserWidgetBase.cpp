

#include "EyelinkUserWidgetBase.h"
#include "EyelinkInterface.h"
#include "EyelinkGameInstance.h"
#include "Engine/UserInterfaceSettings.h"


UEyelinkUserWidgetBase::UEyelinkUserWidgetBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UEyelinkUserWidgetBase::BindEyelinkDelegates(UEyelinkInterface* eyelink)
{
	this->eyelink = eyelink;

	eyelink->SetupCalibrationDisplayDelegate.BindUObject(this, &UEyelinkUserWidgetBase::SetupCalibrationDisplay);
	eyelink->ExitCalibrationDisplayDelegate.BindUObject(this, &UEyelinkUserWidgetBase::ExitCalibrationDisplay);
	eyelink->ClearCalibrationDisplayDelegate.BindUObject(this, &UEyelinkUserWidgetBase::ClearCalibrationDisplay);
	eyelink->EraseCalibrationTargetDelegate.BindUObject(this, &UEyelinkUserWidgetBase::HideCalibrationTarget);
	eyelink->DrawCalibrationTargetDelegate.BindUObject(this, &UEyelinkUserWidgetBase::DrawCalibrationTarget);
}

void UEyelinkUserWidgetBase::UnbindEyelinkDelegates()
{
	if (eyelink)
		eyelink->UnbindDelegates();
}

UEyelinkInterface* UEyelinkUserWidgetBase::GetEyelink() const
{
	UEyelinkGameInstance* gameInstance = Cast<UEyelinkGameInstance>(GetWorld()->GetGameInstance());
	if (gameInstance)
		return gameInstance->Eyelink;
	return nullptr;
}

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