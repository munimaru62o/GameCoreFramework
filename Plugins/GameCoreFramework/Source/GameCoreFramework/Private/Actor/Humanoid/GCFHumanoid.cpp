// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Actor/Humanoid/GCFHumanoid.h"

#include "Components/CapsuleComponent.h"
#include "Movement/Mover/GCFCharacterMoverComponent.h"
#include "MoverTypes.h"
#include "DefaultMovementSet/Settings/StanceSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFHumanoid)


AGCFHumanoid::AGCFHumanoid(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer
			.DoNotCreateDefaultSubobject(AGCFAvatarPawn::AvatarCollisionComponentName))
{
	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("HumanoidCapsuleComponent"));
	if (UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(CollisionComponent)) {
		CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
		CapsuleComp->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
		CapsuleComp->SetMobility(EComponentMobility::Movable);
		RootComponent = CapsuleComp;
	}

	if (USkeletalMeshComponent* SkeletalMesh = GetSkeletalMeshComponent()) {
		SkeletalMesh->SetupAttachment(RootComponent);
		SkeletalMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
		SkeletalMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	}
}


UCapsuleComponent* AGCFHumanoid::GetCapsuleComponent() const
{
	return Cast<UCapsuleComponent>(CollisionComponent);
}


FVector AGCFHumanoid::GetPawnViewLocation() const
{
	// Initialize the offset. The base eye height is handled by the parent class.
	float CrouchOffset = 0.0f;

	// Calculate the downward offset if the character is actively crouching.
	// This logic relies on the Capsule's HalfHeight, making it specific to Humanoid pawns.
	if (MoverComponent && MoverComponent->HasGameplayTag(Mover_IsCrouching, true)) {
		if (const UStanceSettings* StanceSettings = MoverComponent->FindSharedSettings<UStanceSettings>()) {
			if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent()) {
				const float DefaultHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
				CrouchOffset = DefaultHalfHeight - StanceSettings->CrouchHalfHeight;
			}
		}
	}
	// Apply the calculated crouch offset to the base view location provided by the parent class.
	return Super::GetPawnViewLocation() - (FVector::UpVector * CrouchOffset);
}


// --- IGCFHumanoidActionHandler Implementation (Push/Write) ---
void AGCFHumanoid::HandleCrouchInput_Implementation(bool bIsPressed)
{
	// Execute the toggle logic only on the exact frame the button is pressed (Just Pressed).
	if (bIsPressed && !bIsCrouchButtonPressed) {
		bWantsToCrouch = !bWantsToCrouch;
	}

	// Cache the physical button state for the next frame's comparison.
	bIsCrouchButtonPressed = bIsPressed;

	/*
	 * NOTE: If you want to implement "Hold-to-Crouch" instead of "Toggle",
	 * simply replace the above logic with the following:
	 *
	 * bWantsToCrouch = bIsPressed;
	 */
}

// --- IGCFHumanoidActionProvider Implementation (Pull/Read) ---
bool AGCFHumanoid::GetWantsToCrouch_Implementation() const
{
	return bWantsToCrouch;
}