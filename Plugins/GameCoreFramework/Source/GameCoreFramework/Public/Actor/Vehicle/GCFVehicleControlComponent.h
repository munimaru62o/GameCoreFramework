// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Input/GCFInputTypes.h"
#include "System/Lifecycle/GCFStateTypes.h"
#include "Components/PawnComponent.h"
#include "System/Binder/GCFContextBinder.h"
#include "GCFVehicleControlComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class IGCFInputConfigProvider;
class UGCFInputComponent;
struct FInputActionValue;

/**
 * @brief Component handles vehicle-specific input actions (Handbrake, Lights).
 *
 * [Responsibilities]
 * 1. Waits for the Pawn to be fully initialized (Possessed + GameplayReady).
 * 2. Binds Input Actions via the GCF Input System.
 * 3. Forwards commands to the owning AGCFWheeledVehiclePawn.
 *
 * Unlike MovementControlComponent (which handles continuous vectors like steering or throttle),
 * this handles discrete vehicle actions.
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = Pawn, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision, Networking, Replication), meta = (BlueprintSpawnableComponent, CollapseCategories))
class UGCFVehicleControlComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UE_API UGCFVehicleControlComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Checks if the Pawn is ready to receive input, then registers bindings. */
	void HandlePawnReadyStateChanged(const FGCFPawnReadyStateSnapshot& Snapshot);

	/** Registers vehicle input actions (Handbrake, Lights) to the InputComponent. */
	TArray<FGCFBindingReceipt> HandleInputBinding(UGCFInputComponent* InputComponent, TScriptInterface<IGCFInputConfigProvider> Provider);

	// --- Input Handlers ---
	void Input_HandBrake(const FInputActionValue& InputActionValue);
	void Input_Light(const FInputActionValue& InputActionValue);

private:
	/** Binder to observe Pawn readiness. */
	TUniquePtr<FGCFContextBinder> Binder;

	EGCFPawnReadyState CachedPawnReadyState;
};

#undef UE_API