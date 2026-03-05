// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "GCFShared.h"
#include "System/Binder/GCFContextBinder.h"
#include "Movement/GCFMovementTypes.h"
#include "GCFLocomotionDirectionComponent.generated.h"

class UGCFInputComponent;
class IGCFLocomotionInputHandler;
class FGCFContextBinder;
struct FInputActionValue;

/**
 * @brief Handles movement input and configuration for the controlled Pawn.
 *
 * [Responsibilities]
 * 1. Binds movement inputs (Move, MoveUp) and converts them to world-space vectors.
 * 2. Initializes movement settings (speed, acceleration) from PawnData via IGCFMovementConfigReceiver.
 * 3. Adapts movement direction based on the current Camera Policy (e.g., Camera-relative vs World-relative).
 */
UCLASS(ClassGroup = (GCF), Within = Controller, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision, Networking, Replication), meta = (BlueprintSpawnableComponent, CollapseCategories))
class GAMECOREFRAMEWORK_API UGCFLocomotionDirectionComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UGCFLocomotionDirectionComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Input Action Handlers */
	void Input_Move(const FInputActionValue& Value);
	void Input_Move_Completed(const FInputActionValue& Value);
	void Input_MoveUp(const FInputActionValue& Value);
	void Input_MoveUp_Completed(const FInputActionValue& Value);

	/** Calculates the rotation basis for movement input based on the active policy. */
	FRotator CalcMovementRotation(AController* Controller) const;

private:
	/** Called when the Camera Policy changes (e.g., FreeLook <-> Locked). */
	void HandleMovementRotationPolicyChanged(EGCFMovementRotationPolicy Policy);

	void HandlePossessedPawnChanged(AActor* Actor, bool bPossessed);

	/** Binds Input Actions when the Input Component is ready. */
	TArray<FGCFBindingReceipt> HandleInputBinding(UGCFInputComponent* InputComponent, TScriptInterface<IGCFInputConfigProvider> Provider);

private:
	/** Handle for rotation policy delegate. */
	TUniquePtr<FGCFDelegateHandle> Handle;

	// Binder handle to observe pawn possession changes
	TUniquePtr<FGCFContextBinder> PossessionBinder;

	EGCFMovementRotationPolicy CachedPolicy;

	// Cached Pawn to avoid calling GetPawn() every input frame
	UPROPERTY()
	TObjectPtr<APawn> CachedPawn = nullptr;

	// Cached interface pointer to eliminate Implements<U...>() search loop in Hot Path
	UPROPERTY()
	TScriptInterface<IGCFLocomotionInputHandler> CachedLocomotionInputHandler;
};