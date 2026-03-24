

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EyelinkUserWidgetBase.generated.h"

class UEyelinkInterface;
/**
 * A base UMG class for binding eyelink graphics to UI
 */
UCLASS()
class EYELINK_API UEyelinkUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()
public:
	UEyelinkUserWidgetBase(const FObjectInitializer& ObjectInitializer);
	/**
	 * Binds drawing functions to the eyelink object
	 * @param eyelink
	 */
	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void BindEyelinkDelegates(UEyelinkInterface* eyelink);

	UFUNCTION(BlueprintCallable, Category = "Eyelink")
	void UnbindEyelinkDelegates();

	// === Functions to bind to the Eyelink calibration delegates ===
	UFUNCTION(BlueprintImplementableEvent, Category = "Eyelink", meta = (ForceAsFunction))
	int SetupCalibrationDisplay();

	UFUNCTION(BlueprintImplementableEvent, Category = "Eyelink", meta = (ForceAsFunction))
	void ExitCalibrationDisplay();

	UFUNCTION(BlueprintImplementableEvent, Category = "Eyelink", meta = (ForceAsFunction))
	void ClearCalibrationDisplay();

	UFUNCTION(BlueprintImplementableEvent, Category = "Eyelink", meta = (ForceAsFunction))
	void HideCalibrationTarget();

	UFUNCTION(BlueprintImplementableEvent, Category = "Eyelink", meta = (ForceAsFunction))
	void DrawCalibrationTarget(int x, int y);

	UFUNCTION(BlueprintPure, Category = "Eyelink")
	UEyelinkInterface* GetEyelink() const;

	UFUNCTION(BlueprintPure, Category = "Eyelink")
	float GetDPIScaleFactor() const;

private:
	UEyelinkInterface* eyelink;	// reference to eyelink object	
};
