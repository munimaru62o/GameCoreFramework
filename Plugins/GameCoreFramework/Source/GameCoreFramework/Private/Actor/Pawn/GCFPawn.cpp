// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 62o. All rights reserved.

#include "Actor/Pawn/GCFPawn.h"
#include "Actor/Data/GCFPawnData.h"
#include "System/Lifecycle/GCFPawnExtensionComponent.h"
#include "Camera/GCFCameraComponent.h"
#include "Components/SphereComponent.h"
#include "System/Lifecycle/GCFPawnReadyStateComponent.h"
#include "Input/GCFPawnInputBridgeComponent.h"
#include "MoverComponent.h"


AGCFPawn::AGCFPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	NetDormancy = DORM_Never;
	bReplicates = true;

	// Create the collision sphere and set it as Root.
	// This enables basic collision detection for floating movement.
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(32.0f);
	CollisionComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	CollisionComponent->SetMobility(EComponentMobility::Movable);
	RootComponent = CollisionComponent;

	PawnExtensionComponent = CreateDefaultSubobject<UGCFPawnExtensionComponent>(TEXT("GCFPawnExtensionComponent"));
	PawnReadyStateComponent = CreateDefaultSubobject<UGCFPawnReadyStateComponent>(TEXT("GCFPawnReadyStateComponent"));
	PawnInputBridgeComponent = CreateDefaultSubobject<UGCFPawnInputBridgeComponent>(TEXT("PawnInputBridgeComponent"));

	CameraComponent = CreateDefaultSubobject<UGCFCameraComponent>(TEXT("GCFCameraComponent"));
	CameraComponent->SetupAttachment(CollisionComponent);
	CameraComponent->bUsePawnControlRotation = false; // Camera rotation is handled by the CameraComponent/Modes.
}


void AGCFPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}


const UGCFPawnData* AGCFPawn::GetPawnData() const
{
	if (PawnExtensionComponent) {
		return PawnExtensionComponent->GetPawnData<UGCFPawnData>();
	}
	return nullptr;
}


void AGCFPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Notify extension component on Server side possession.
	if (PawnExtensionComponent) {
		PawnExtensionComponent->OnControllerAssigned();
	}
}


void AGCFPawn::UnPossessed()
{
	Super::UnPossessed();

	// Notify extension component to clean up or reset state.
	if (PawnExtensionComponent) {
		PawnExtensionComponent->HandleControllerChanged();
	}
}


void AGCFPawn::OnRep_Controller()
{
	Super::OnRep_Controller();

	// Notify extension component on Client side when the Controller reference replicates.
	if (PawnExtensionComponent) {
		PawnExtensionComponent->OnControllerAssigned();
	}
}


void AGCFPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Notify extension component on Client side when the PlayerState reference replicates.
	// This is crucial for initializing AbilitySystem or HUD that depends on PlayerState data.
	if (PawnExtensionComponent) {
		PawnExtensionComponent->HandlePlayerStateReplicated();
	}
}


void AGCFPawn::HandleMoveInput_Implementation(const FVector2D& InputValue, const FRotator& MovementRotation)
{
	if (bUseMoverComponent) {
		// Cache the input state for the Mover's tick-based polling system
		CachedMoveInput = InputValue;
		CachedMoveRotation = MovementRotation;
		UpdateCachedTargetMovement();
	} else {
		// Traditional immediate input processing (e.g., for FloatingPawnMovement)
		if (InputValue.X != 0.0f) {
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(MovementDirection, InputValue.X);
		}

		if (InputValue.Y != 0.0f) {
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			AddMovementInput(MovementDirection, InputValue.Y);
		}
	}
}


void AGCFPawn::HandleMoveUpInput_Implementation(float Value)
{
	if (bUseMoverComponent) {
		// Cache the vertical input state
		CachedUpInput = Value;
		UpdateCachedTargetMovement();
	} else {
		AddMovementInput(FVector::UpVector, Value);
	}
}


void AGCFPawn::UpdateCachedTargetMovement()
{
	// Horizontal Movement Calculation (Applied with Camera/Controller rotation)
	const FVector ForwardDir = CachedMoveRotation.RotateVector(FVector::ForwardVector) * CachedMoveInput.X;
	const FVector RightDir = CachedMoveRotation.RotateVector(FVector::RightVector) * CachedMoveInput.Y;

	// Vertical Movement Calculation (Absolute World Z-Axis)
	const FVector UpDir = FVector::UpVector * CachedUpInput;

	// Composite and store the final intent (The Producer will read this cached value)
	CachedTargetMovement = ForwardDir + RightDir + UpDir;
}


FVector AGCFPawn::GetDesiredMovementVector_Implementation() const
{
	return CachedTargetMovement;
}


void AGCFPawn::OnDeathStarted(AActor*)
{
	// Disable collision and movement here if needed.
	if (CollisionComponent) {
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}


void AGCFPawn::OnDeathFinished(AActor*)
{
	// Detach controller and destroy pawn logic.
	DetachFromControllerPendingDestroy();
	SetLifeSpan(0.1f);
}