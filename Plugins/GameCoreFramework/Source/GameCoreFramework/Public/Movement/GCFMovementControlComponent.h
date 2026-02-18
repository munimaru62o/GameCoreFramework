// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "GCFShared.h"
#include "System/Binder/GCFContextBinder.h"
#include "Movement/GCFMovementTypes.h"
#include "GCFMovementControlComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

enum class EGCFPawnReadyState : uint8;
class UGCFInputComponent;
class IGCFPawnDataProvider;
class UGCFMovementConfig;
struct FInputActionValue;

/**
 * @brief Handles movement input and configuration for the controlled Pawn.
 *
 * [Responsibilities]
 * 1. Binds movement inputs (Move, MoveUp) and converts them to world-space vectors.
 * 2. Initializes movement settings (speed, acceleration) from PawnData via IGCFMovementConfigReceiver.
 * 3. Adapts movement direction based on the current Camera Policy (e.g., Camera-relative vs World-relative).
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = Controller, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision, Networking, Replication), meta = (BlueprintSpawnableComponent, CollapseCategories))
class UGCFMovementControlComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UE_API UGCFMovementControlComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Binds Input Actions when the Input Component is ready. */
	TArray<FGCFBindingReceipt> HandleInputBinding(UGCFInputComponent* InputComponent, TScriptInterface<IGCFInputConfigProvider> Provider);

	/** Called when the Camera Policy changes (e.g., FreeLook <-> Locked). */
	void HandleMovementRotationPolicyChanged(EGCFMovementRotationPolicy Policy);

	/** Input Action Handlers */
	void Input_Move(const FInputActionValue& Value);
	void Input_Move_Completed(const FInputActionValue& Value);
	void Input_MoveUp(const FInputActionValue& Value);
	void Input_MoveUp_Completed(const FInputActionValue& Value);

	/** Calculates the rotation basis for movement input based on the active policy. */
	FRotator CalcMovementRotation(AController* Controller) const;

private:
	/** Handle for rotation policy delegate. */
	TUniquePtr<FGCFDelegateHandle> Handle;

	EGCFMovementRotationPolicy CachedPolicy;
};

#undef UE_API