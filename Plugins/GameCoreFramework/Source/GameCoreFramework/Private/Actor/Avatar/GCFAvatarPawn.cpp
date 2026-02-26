// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Actor/Avatar/GCFAvatarPawn.h"

#include "GCFShared.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Actor/Avatar/GCFAvatarControlComponent.h"
#include "Movement/Mover/GCFCharacterMoverComponent.h"
#include "MoverTypes.h"
#include "DefaultMovementSet/Settings/StanceSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFAvatarPawn)

class AActor;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UInputComponent;


AGCFAvatarPawn::AGCFAvatarPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer
			// Prevent the base class from creating generic Sphere and StaticMesh components.
			// We use specialized Capsule and SkeletalMesh components for humanoids instead.
			.DoNotCreateDefaultSubobject(Super::MeshComponentName)
			.DoNotCreateDefaultSubobject(Super::CollisionComponentName))
{
	// Avoid ticking characters if possible.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	CapsuleComp->SetMobility(EComponentMobility::Movable);
	RootComponent = CapsuleComp;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	USkeletalMeshComponent* SkeletalMesh = GetSkeletalMeshComponent();
	check(SkeletalMesh);
	SkeletalMesh->SetupAttachment(RootComponent);
	SkeletalMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));  // Rotate mesh to be X forward since it is exported as Y forward.
	SkeletalMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));

	AvatarControlComponent = CreateDefaultSubobject<UGCFAvatarControlComponent>(TEXT("AvatarControlComponent"));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	BaseEyeHeight = 80.0f;
	SetNetCullDistanceSquared(900000000.0f);

	// Note: MoverComponent and InputProducerComponent are intentionally omitted from this constructor.
	// Following the Composition over Inheritance principle, these should be added via Blueprints
	// (e.g., BP_StandardAvatarMover) to allow designers full control over shared settings and movement modes.
}


UCapsuleComponent* AGCFAvatarPawn::GetCapsuleComponent() const
{
	return Cast<UCapsuleComponent>(CollisionComponent);
}


USkeletalMeshComponent* AGCFAvatarPawn::GetSkeletalMeshComponent() const
{
	return Cast<USkeletalMeshComponent>(MeshComponent);
}


UGCFCharacterMoverComponent* AGCFAvatarPawn::GetCharacterMoverComponent() const
{
	return Cast<UGCFCharacterMoverComponent>(MoverComponent);
}


void AGCFAvatarPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}


void AGCFAvatarPawn::BeginPlay()
{
	Super::BeginPlay();

	// MoverComponentのスタンス変更イベントにバインドする
	/*if (UCharacterMoverComponent* CharMover = GetCharacterMoverComponent()) {
		CharMover->OnStanceChanged.AddDynamic(this, &ThisClass::HandleStanceChanged);
	}*/
}


void AGCFAvatarPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}


FVector AGCFAvatarPawn::GetPawnViewLocation() const
{
	// Get the base eye height defined in the class.
	float TargetEyeHeight = BaseEyeHeight;

	// If the Mover is currently in a "Crouching" state, lower the eye height.
	if (MoverComponent) {
		if (USceneComponent* VisualComp = MoverComponent->GetPrimaryVisualComponent()) {
			if (MoverComponent->HasGameplayTag(Mover_IsCrouching, true)) {
				if (const UStanceSettings* StanceSettings = MoverComponent->FindSharedSettings<UStanceSettings>()) {
					const float DefaultHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
					const float CrouchOffset = DefaultHalfHeight - StanceSettings->CrouchHalfHeight;
					TargetEyeHeight -= CrouchOffset;
				}
			}
			return VisualComp->GetComponentLocation() + (FVector::UpVector * TargetEyeHeight);
		}
	}

	// Return the actor's world location offset by the calculated eye height.
	return GetActorLocation() + (FVector::UpVector * TargetEyeHeight);
}


// --- IGCFAvatarActionHandler Implementation (Push/Write) ---
void AGCFAvatarPawn::HandleJumpInput_Implementation(bool bIsPressed)
{
	// Detect the exact frame the button was pressed (JustPressed)
	if (bIsPressed && !bIsJumpPressed) {
		bIsJumpJustPressed = true;
	}

	// Update the continuous hold state
	bIsJumpPressed = bIsPressed;
}

void AGCFAvatarPawn::HandleCrouchInput_Implementation(bool bIsPressed)
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


// --- IGCFAvatarActionProvider Implementation (Pull/Read) ---
bool AGCFAvatarPawn::GetIsJumpPressed_Implementation() const
{
	return bIsJumpPressed;
}

bool AGCFAvatarPawn::GetIsJumpJustPressed_Implementation() const
{
	return bIsJumpJustPressed;
}

void AGCFAvatarPawn::ConsumeJumpJustPressed_Implementation()
{
	bIsJumpJustPressed = false;
}

bool AGCFAvatarPawn::GetWantsToCrouch_Implementation() const
{
	return bWantsToCrouch;
}


void AGCFAvatarPawn::HandleStanceChanged(EStanceMode OldStance, EStanceMode NewStance)
{
	UMeshComponent* MeshComp = GetMeshComponent();
	if (!MeshComp || !MoverComponent) return;

	const UStanceSettings* StanceSettings = MoverComponent->FindSharedSettings<UStanceSettings>();
	if (!StanceSettings) return;

	const float DefaultHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float CrouchOffset = DefaultHalfHeight - StanceSettings->CrouchHalfHeight;

	if (NewStance == EStanceMode::Crouch) {
		AddActorWorldOffset(FVector(0.0f, 0.0f, -CrouchOffset));
		MeshComp->AddLocalOffset(FVector(0.0f, 0.0f, CrouchOffset));
	} else if (OldStance == EStanceMode::Crouch && NewStance == EStanceMode::Invalid) {
		AddActorWorldOffset(FVector(0.0f, 0.0f, CrouchOffset));
		MeshComp->AddLocalOffset(FVector(0.0f, 0.0f, -CrouchOffset));
	}
}