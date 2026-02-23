// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Animation/GCFHumanoidAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "MoverComponent.h"
#include "MoverTypes.h"
#include "GameplayTagAssetInterface.h"
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

		FMoverInputCmdContext LastInput = MoverComponent->GetLastInputCmd();
		if (const FCharacterDefaultInputs* CharacterInputs = LastInput.InputCollection.FindDataByType<FCharacterDefaultInputs>()) {
			bHasAcceleration = !CharacterInputs->GetMoveInput().IsNearlyZero(0.01f);
		} else {
			bHasAcceleration = false;
		}

		// Movement Mode
		CurrentMovementMode = MoverComponent->GetMovementModeName();

		// Determine Falling state
		bIsFalling = (CurrentMovementMode == TEXT("Falling"));
	}

	// Since Mover doesn't natively have "Crouch" bool, we rely on the tag synced by the Pawn/Mover bridge.
	if (const IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OwningPawn)) {
		bIsCrouched = TagInterface->HasMatchingGameplayTag(GCFGameplayTags::Status_Crouching);
	} else {
		bIsCrouched = false;
	}
}