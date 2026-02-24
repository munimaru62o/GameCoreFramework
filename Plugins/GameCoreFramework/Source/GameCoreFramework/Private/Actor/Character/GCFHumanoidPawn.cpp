// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Actor/Character/GCFHumanoidPawn.h"

#include "GCFShared.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Actor/Character/GCFCharacterControlComponent.h"
#include "Movement/Mover/GCFCharacterMoverComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFHumanoidPawn)

class AActor;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UInputComponent;


AGCFHumanoidPawn::AGCFHumanoidPawn(const FObjectInitializer& ObjectInitializer)
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

	CharacterControlComponent = CreateDefaultSubobject<UGCFCharacterControlComponent>(TEXT("CharacterControlComponent"));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	BaseEyeHeight = 80.0f;
	SetNetCullDistanceSquared(900000000.0f);

	// TODO: Implement crouched eye height adjustments.
	//CrouchedEyeHeight = 50.0f;

	// Note: MoverComponent and InputProducerComponent are intentionally omitted from this constructor.
	// Following the Composition over Inheritance principle, these should be added via Blueprints
	// (e.g., BP_StandardHumanoidMover) to allow designers full control over shared settings and movement modes.
}


UCapsuleComponent* AGCFHumanoidPawn::GetCapsuleComponent() const
{
	return Cast<UCapsuleComponent>(CollisionComponent);
}


USkeletalMeshComponent* AGCFHumanoidPawn::GetSkeletalMeshComponent() const
{
	return Cast<USkeletalMeshComponent>(MeshComponent);
}


UGCFCharacterMoverComponent* AGCFHumanoidPawn::GetCharacterMoverComponent() const
{
	return Cast<UGCFCharacterMoverComponent>(MoverComponent);
}


void AGCFHumanoidPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}


void AGCFHumanoidPawn::BeginPlay()
{
	Super::BeginPlay();
}


void AGCFHumanoidPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}


void AGCFHumanoidPawn::ToggleCrouch()
{
	bWantsToCrouch = !bWantsToCrouch;
}


void AGCFHumanoidPawn::Jump()
{
	bWantsToJump = true;
}


bool AGCFHumanoidPawn::CanJump() const
{
	return !bWantsToJump;
}


bool AGCFHumanoidPawn::GetWantsToCrouch() const
{
	return bWantsToCrouch;
}


bool AGCFHumanoidPawn::GetWantsToJump() const
{
	return bWantsToJump;
}