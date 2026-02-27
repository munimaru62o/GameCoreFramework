// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Actor/Avatar/GCFAvatarPawn.h"

#include "GCFShared.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Movement/Mover/GCFCharacterMoverComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFAvatarPawn)

class AActor;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UInputComponent;

const FName AGCFAvatarPawn::AvatarCollisionComponentName(TEXT("AvatarCollisionComponent"));
const FName AGCFAvatarPawn::AvatarMeshComponentName(TEXT("AvatarMeshComponent"));


AGCFAvatarPawn::AGCFAvatarPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer
			// Prevent the base class from creating generic Sphere and StaticMesh components.
			// We use specialized Capsule and SkeletalMesh components for humanoids instead.
			.DoNotCreateDefaultSubobject(AGCFPawn::CollisionComponentName)
			.DoNotCreateDefaultSubobject(AGCFPawn::MeshComponentName))
{
	// Avoid ticking characters if possible.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	CollisionComponent = CreateOptionalDefaultSubobject<USphereComponent>(AvatarCollisionComponentName);
	if (USphereComponent* SphereComp = Cast<USphereComponent>(CollisionComponent)) {
		SphereComp->InitSphereRadius(40.0f);
		SphereComp->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
		SphereComp->SetMobility(EComponentMobility::Movable);
		RootComponent = SphereComp;
	}

	MeshComponent = CreateOptionalDefaultSubobject<USkeletalMeshComponent>(AvatarMeshComponentName);
	if (USkeletalMeshComponent* SkeletalMesh = GetSkeletalMeshComponent()) {
		SkeletalMesh->SetupAttachment(RootComponent);
		SkeletalMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));  // Rotate mesh to be X forward since it is exported as Y forward.
		SkeletalMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	}

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Disable standard actor movement replication at the CDO level.
	// This prevents Blueprint validation errors, as Mover handles its own Network Prediction synchronization.
	SetReplicateMovement(false);
	bUseMoverComponent = true;

	BaseEyeHeight = 80.0f;
	SetNetCullDistanceSquared(900000000.0f);

	// Note: MoverComponent and InputProducerComponent are intentionally omitted from this constructor.
	// Following the Composition over Inheritance principle, these should be added via Blueprints
	// (e.g., BP_StandardAvatarMover) to allow designers full control over shared settings and movement modes.
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
}


void AGCFAvatarPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}


// --- Input Handlers (Push/Write) ---
void AGCFAvatarPawn::HandleJumpInput_Implementation(bool bIsPressed)
{
	// Detect the exact frame the button was pressed (JustPressed)
	if (bIsPressed && !bIsJumpPressed) {
		bIsJumpJustPressed = true;
	}

	// Update the continuous hold state
	bIsJumpPressed = bIsPressed;
}

// --- Input Providers (Pull/Read) ---
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