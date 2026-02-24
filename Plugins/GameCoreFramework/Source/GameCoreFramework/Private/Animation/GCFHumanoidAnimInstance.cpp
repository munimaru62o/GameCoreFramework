// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Animation/GCFHumanoidAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "MoverComponent.h"
#include "MoverTypes.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "GCFShared.h"


UGCFHumanoidAnimInstance::UGCFHumanoidAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GroundSpeed = 0.0f;
	VerticalVelocity = 0.0f;
	bHasAcceleration = false;
	bIsFalling = false;
	bIsCrouched = false;
	CurrentMovementMode = NAME_None;
}

void UGCFHumanoidAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Cache the owning pawn
	OwningPawn = TryGetPawnOwner();
	if (OwningPawn) {
		MoverComponent = OwningPawn->FindComponentByClass<UMoverComponent>();
	}
}

void UGCFHumanoidAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningPawn) {
		return;
	}

	// If Mover is not found yet, try to find it again (handle late initialization)
	if (!MoverComponent) {
		MoverComponent = OwningPawn->FindComponentByClass<UMoverComponent>();
	}

	// --- Update Logic based on Mover ---
	if (MoverComponent) {
		// Velocity
		Velocity = MoverComponent->GetVelocity();
		VerticalVelocity = Velocity.Z;
		GroundSpeed = Velocity.Size2D();
		MovementDirection = CalculateDirection(Velocity, OwningPawn->GetActorRotation());

		bShouldMove = GroundSpeed > 3.0f;
		bHasAcceleration = HasAcceleration();

		// Movement Mode
		CurrentMovementMode = MoverComponent->GetMovementModeName();

		// Determine Falling state using Native Gameplay Tags instead of hardcoded strings or class casting.
		// This guarantees that any custom movement mode (e.g., Jetpack, Grapple) that applies the 
		// "Falling" or "InAir" tag will automatically trigger the correct airborne animation.
		bIsFalling = MoverComponent->HasGameplayTag(Mover_IsFalling, true);

		// Retrieve crouch state directly via Native Gameplay Tags to avoid hard casting.
		// The Epic standard StanceModifier automatically applies the "Mover.IsCrouching" tag.
		bIsCrouched = MoverComponent->HasGameplayTag(Mover_IsCrouching, true);
	}
}


bool UGCFHumanoidAnimInstance::HasAcceleration() const
{
	if (!MoverComponent || !OwningPawn) {
		return false;
	}

	// For simulated proxies (other players), read the intent from the server-replicated SyncState.
	if (OwningPawn->GetLocalRole() == ROLE_SimulatedProxy) {
		if (const FMoverDefaultSyncState* SyncState = MoverComponent->GetSyncState().SyncStateCollection.FindDataByType<FMoverDefaultSyncState>()) {
			return !SyncState->MoveDirectionIntent.IsNearlyZero(0.01f);
		}
	}
	// For local/authority (this player), read the intent directly from the most recent InputCmd.
	else {
		FMoverInputCmdContext LastInput = MoverComponent->GetLastInputCmd();
		if (const FCharacterDefaultInputs* CharacterInputs = LastInput.InputCollection.FindDataByType<FCharacterDefaultInputs>()) {
			return !CharacterInputs->GetMoveInput().IsNearlyZero(0.01f);
		}
	}
	return false;
}