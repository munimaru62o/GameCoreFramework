// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Animation/GCFAvatarAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "MoverComponent.h"
#include "MoverTypes.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "GCFShared.h"
#include "KismetAnimationLibrary.h"


UGCFAvatarAnimInstance::UGCFAvatarAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GroundSpeed = 0.0f;
	VerticalVelocity = 0.0f;
	bHasAcceleration = false;
	bIsFalling = false;
	bIsCrouched = false;
	CurrentMovementMode = NAME_None;
}

void UGCFAvatarAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Cache the owning pawn
	OwningPawn = TryGetPawnOwner();
	if (OwningPawn) {
		MoverComponent = OwningPawn->FindComponentByClass<UMoverComponent>();
	}
}

void UGCFAvatarAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningPawn) {
		return;
	}

	// If Mover is not found yet, try to find it again (handle late initialization)
	if (!MoverComponent) {
		MoverComponent = OwningPawn->FindComponentByClass<UMoverComponent>();
	}

	// ------------------------------------------------------------------------
	// [GAME THREAD]
	// Safely gather raw data from Actors and Components here.
	// Do NOT perform heavy math operations in this function.
	// ------------------------------------------------------------------------
	if (MoverComponent) {
		Velocity = MoverComponent->GetVelocity();
		CachedActorRotation = OwningPawn->GetActorRotation();

		bHasAcceleration = HasAcceleration();
		CurrentMovementMode = MoverComponent->GetMovementModeName();

		// Determine Falling state using Native Gameplay Tags instead of hardcoded strings or class casting.
		bIsFalling = MoverComponent->HasGameplayTag(Mover_IsFalling, true);

		// Retrieve crouch state directly via Native Gameplay Tags to avoid hard casting.
		bIsCrouched = MoverComponent->HasGameplayTag(Mover_IsCrouching, true);
	}
}


void UGCFAvatarAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	// ------------------------------------------------------------------------
	// [WORKER THREAD]
	// Perform mathematical calculations using ONLY the cached variables.
	// Do NOT call functions on Actors or Components (like GetActorRotation) here.
	// ------------------------------------------------------------------------

	VerticalVelocity = Velocity.Z;
	GroundSpeed = Velocity.Size2D();

	// Safely calculate direction using the cached rotation
	MovementDirection = UKismetAnimationLibrary::CalculateDirection(Velocity, CachedActorRotation);

	bShouldMove = GroundSpeed > 3.0f;
}


bool UGCFAvatarAnimInstance::HasAcceleration() const
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