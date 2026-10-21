// Copyright Epic Games, Inc. All Rights Reserved.


#include "ExamplePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Components/InputComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Example.h"
#include "Misc/App.h"
#include "NavigationSystem.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Input/SVirtualJoystick.h"

AExamplePlayerController::AExamplePlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Content references previously held by the Blueprint controller subclass.
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> defaultContextFinder(TEXT("/Game/Input/IMC_Default"));
	if (defaultContextFinder.Succeeded())
		DefaultMappingContexts.Add(defaultContextFinder.Object);

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> mouseLookContextFinder(TEXT("/Game/Input/IMC_MouseLook"));
	if (mouseLookContextFinder.Succeeded())
		MobileExcludedMappingContexts.Add(mouseLookContextFinder.Object);

	static ConstructorHelpers::FClassFinder<UUserWidget> touchControlsFinder(TEXT("/Game/Input/Touch/UI_TouchSimple"));
	if (touchControlsFinder.Succeeded())
		MobileControlsWidgetClass = touchControlsFinder.Class;

	// GameHUD, SettingsHUD, and BP_TrialItem load lazily at first use.
	// GameHUD references this class and the spectator controller, so loading it
	// from a constructor re-enters the loader on startup and deadlocks it.
}

void AExamplePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// The UIOnly input mode set by the menus lives on the game viewport, and
	// the fixed time step used for replay rendering is engine-global; both
	// survive map reloads, so reclaim normal input and time for gameplay.
	if (IsLocalPlayerController())
	{
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);
		FApp::SetUseFixedTimeStep(false);
		FApp::SetBenchmarking(false);
		FApp::SetFixedDeltaTime(0.0);
	}

	// Create the gameplay HUD. The widget binds itself to this controller's
	// display delegates in its Construct event.
	if (!GameHUDClass)
		GameHUDClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/Game/Blueprints/Game/GameHUD.GameHUD_C"));
	if (IsLocalPlayerController() && GameHUDClass)
	{
		UUserWidget* gameHUD = CreateWidget<UUserWidget>(this, GameHUDClass);
		if (gameHUD)
			gameHUD->AddToViewport();
	}

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogExample, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AExamplePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}

	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AExamplePlayerController::HandleEscapePressed);
	InputComponent->BindKey(EKeys::M, IE_Pressed, this, &AExamplePlayerController::HandleEscapePressed);
}

void AExamplePlayerController::HandleEscapePressed()
{
	if (SettingsHUDWidget && SettingsHUDWidget->IsInViewport())
		return;

	if (!SettingsHUDWidget)
	{
		if (!SettingsHUDClass)
			SettingsHUDClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/Game/Blueprints/Game/SettingsHUD.SettingsHUD_C"));
		if (!SettingsHUDClass)
			return;
		SettingsHUDWidget = CreateWidget<UUserWidget>(this, SettingsHUDClass);
		if (!SettingsHUDWidget)
			return;
	}

	OpenSettingsMenu();
}

void AExamplePlayerController::OpenSettingsMenu()
{
	SettingsHUDWidget->AddToViewport();
	SetShowMouseCursor(true);

	// Focus the menu so its OnKeyDown receives the Escape that closes it
	SettingsHUDWidget->SetIsFocusable(true);
	FInputModeUIOnly inputMode;
	inputMode.SetWidgetToFocus(SettingsHUDWidget->TakeWidget());
	SetInputMode(inputMode);
}

void AExamplePlayerController::ExperimentTick(float deltaTime)
{
	// Main task loop: a collection task. Each trial spawns 1-3 boxes at random
	// points on the navmesh; walking into a box collects it for one point.

	if (!bTrialActive)
	{
		SecondsUntilNextTrial -= deltaTime;
		if (SecondsUntilNextTrial <= 0.0)
			StartTrial();
		return;
	}

	TrialElapsedSeconds += deltaTime;
	if ((int)TrialElapsedSeconds != LastReportedTrialSecond)
	{
		LastReportedTrialSecond = (int)TrialElapsedSeconds;
		SetTimeRemainingDelegate.ExecuteIfBound(LastReportedTrialSecond);
	}

	APawn* pawn = GetPawn();
	if (!pawn)
		return;

	for (int itemIndex = TrialItems.Num() - 1; itemIndex >= 0; itemIndex--)
	{
		if (TrialItems[itemIndex] && TrialItems[itemIndex]->IsOverlappingActor(pawn))
			CollectItem(itemIndex);
	}
}

void AExamplePlayerController::ResetExperimentState()
{
	Super::ResetExperimentState();

	for (AStaticMeshActor* item : TrialItems)
		if (item)
			item->Destroy();
	TrialItems.Empty();

	ItemsToCollect = 0;
	ItemsCollected = 0;
	TrialElapsedSeconds = 0.0;
	LastReportedTrialSecond = -1;
	SecondsUntilNextTrial = 0.0;
	bTrialActive = false;
}

bool AExamplePlayerController::GetDirectionToNearestItem(float& angleDegrees) const
{
	angleDegrees = 0.0f;

	APawn* pawn = GetPawn();
	if (!bTrialActive || !pawn || TrialItems.Num() == 0)
		return false;

	const FVector pawnLocation = pawn->GetActorLocation();
	double nearestDistanceSquared = TNumericLimits<double>::Max();
	FVector nearestLocation = FVector::ZeroVector;
	bool found = false;

	for (const AStaticMeshActor* item : TrialItems)
	{
		if (!item)
			continue;
		const double distanceSquared = FVector::DistSquared(item->GetActorLocation(), pawnLocation);
		if (distanceSquared < nearestDistanceSquared)
		{
			nearestDistanceSquared = distanceSquared;
			nearestLocation = item->GetActorLocation();
			found = true;
		}
	}
	if (!found)
		return false;

	const double worldYaw = (nearestLocation - pawnLocation).Rotation().Yaw;
	angleDegrees = (float)FRotator::NormalizeAxis(worldYaw - GetControlRotation().Yaw);
	return true;
}

void AExamplePlayerController::StartTrial()
{
	APawn* pawn = GetPawn();
	UNavigationSystemV1* navSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!pawn || !navSystem)
		return;	// bTrialActive stays false, so the next tick retries

	ItemsToCollect = randomStream.RandRange(1, 3);
	ItemsCollected = 0;
	TrialElapsedSeconds = 0.0;
	LastReportedTrialSecond = -1;
	TrialItems.Empty();

	for (int itemIndex = 0; itemIndex < ItemsToCollect; itemIndex++)
	{
		FNavLocation navLocation;
		if (!navSystem->GetRandomReachablePointInRadius(pawn->GetActorLocation(), ItemSpawnRadius, navLocation))
			continue;

		AStaticMeshActor* item = SpawnItem(navLocation.Location);
		if (item)
			TrialItems.Add(item);
	}

	// A navmesh query can fail; the trial runs with the items that spawned
	ItemsToCollect = TrialItems.Num();
	if (ItemsToCollect == 0)
		return;

	bTrialActive = true;
	SetDisplayText(FString::Printf(TEXT("Collect %d items"), ItemsToCollect), 2.0);
}

void AExamplePlayerController::CollectItem(int itemIndex)
{
	AStaticMeshActor* item = TrialItems[itemIndex];
	TrialItems.RemoveAt(itemIndex);
	if (item)
		item->Destroy();

	ItemsCollected++;
	UpdatePoints();

	if (ItemsCollected >= ItemsToCollect)
	{
		SetDisplayText(FString(TEXT("All Collected")), 2.0);
		bTrialActive = false;
		SecondsUntilNextTrial = SecondsBetweenTrials;
	}
	else
	{
		SetDisplayText(FString::Printf(TEXT("Collected %d/%d"), ItemsCollected, ItemsToCollect), 2.0);
	}
}

AStaticMeshActor* AExamplePlayerController::SpawnItem(const FVector& location)
{
	if (!ItemClass)
		ItemClass = StaticLoadClass(AStaticMeshActor::StaticClass(), nullptr, TEXT("/Game/Blueprints/BP_TrialItem.BP_TrialItem_C"));
	if (!ItemClass)
		return nullptr;

	FActorSpawnParameters spawnParameters;
	spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// The cube mesh is 100 units; place the box center half a box above the navmesh
	return GetWorld()->SpawnActor<AStaticMeshActor>(ItemClass,
		location + FVector(0.0, 0.0, 50.0), FRotator::ZeroRotator, spawnParameters);
}

bool AExamplePlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
