// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#include "Movement/GCFCharacterMovementComponent.h"

#include "GCFShared.h"
#include "Movement/GCFMovementConfig.h"
#include "Actor/Character/GCFCharacter.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystem/GCFAbilitySystemFunctionLibrary.h"
#include "AbilitySystem/GCFAbilitySystemComponent.h"

namespace 
{
static float GroundTraceDistance = 100000.0f;
}



UGCFCharacterMovementComponent::UGCFCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bRunPhysicsWithNoController = true;
}


void UGCFCharacterMovementComponent::ApplyMovementConfig_Implementation(const UGCFMovementConfig* Config)
{
	if (!Config) {
		return;
	}

	// Apply Base Settings
	MaxWalkSpeed = Config->MaxSpeed;
	MaxAcceleration = Config->Acceleration;
}


void UGCFCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// Attempt to cache the ASC immediately on startup.
	// Note: It might still be null here if the ASC is initialized later (e.g. on PlayerState).
	CachedAbilitySystemComponent = UGCFAbilitySystemFunctionLibrary::GetPawnAbilitySystemComponent(Cast<APawn>(GetOwner()));
}


void UGCFCharacterMovementComponent::SimulateMovement(float DeltaTime)
{
	if (bHasReplicatedAcceleration) {
		// Preserve our replicated acceleration
		const FVector OriginalAcceleration = Acceleration;
		Super::SimulateMovement(DeltaTime);
		Acceleration = OriginalAcceleration;
	} else {
		Super::SimulateMovement(DeltaTime);
	}
}


bool UGCFCharacterMovementComponent::CanAttemptJump() const
{
	// Same as UCharacterMovementComponent's implementation but without the crouch check
	return IsJumpAllowed() &&
		(IsMovingOnGround() || IsFalling()); // Falling included for double-jump and non-zero jump hold time, but validated by character.
}


const FGCFCharacterGroundInfo& UGCFCharacterMovementComponent::GetGroundInfo()
{
	if (!CharacterOwner || (GFrameCounter == CachedGroundInfo.LastUpdateFrame)) {
		return CachedGroundInfo;
	}

	if (MovementMode == MOVE_Walking) {
		CachedGroundInfo.GroundHitResult = CurrentFloor.HitResult;
		CachedGroundInfo.GroundDistance = 0.0f;
	} else {
		const UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
		check(CapsuleComp);

		const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
		const ECollisionChannel CollisionChannel = (UpdatedComponent ? UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
		const FVector TraceStart(GetActorLocation());
		const FVector TraceEnd(TraceStart.X, TraceStart.Y, (TraceStart.Z - GroundTraceDistance - CapsuleHalfHeight));

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GCFCharacterMovementComponent_GetGroundInfo), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(QueryParams, ResponseParam);

		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParam);

		CachedGroundInfo.GroundHitResult = HitResult;
		CachedGroundInfo.GroundDistance = GroundTraceDistance;

		if (MovementMode == MOVE_NavWalking) {
			CachedGroundInfo.GroundDistance = 0.0f;
		} else if (HitResult.bBlockingHit) {
			CachedGroundInfo.GroundDistance = FMath::Max((HitResult.Distance - CapsuleHalfHeight), 0.0f);
		}
	}

	CachedGroundInfo.LastUpdateFrame = GFrameCounter;

	return CachedGroundInfo;
}


void UGCFCharacterMovementComponent::SetReplicatedAcceleration(const FVector& InAcceleration)
{
	bHasReplicatedAcceleration = true;
	Acceleration = InAcceleration;
}


FRotator UGCFCharacterMovementComponent::GetDeltaRotation(float DeltaTime) const
{
	// Check cached ASC. If null, try to lazy load it (safe because it's mutable).
	if (!CachedAbilitySystemComponent.IsValid()) {
		CachedAbilitySystemComponent = UGCFAbilitySystemFunctionLibrary::GetPawnAbilitySystemComponent(Cast<APawn>(GetOwner()));
	}

	if (UAbilitySystemComponent* ASC = CachedAbilitySystemComponent.Get()) {
		if (ASC->HasMatchingGameplayTag(GCFGameplayTags::Gameplay_Movement_Stop)) {
			return FRotator(0, 0, 0);
		}
	}
	return Super::GetDeltaRotation(DeltaTime);
}


float UGCFCharacterMovementComponent::GetMaxSpeed() const
{
	// Check cached ASC. If null, try to lazy load it.
	if (!CachedAbilitySystemComponent.IsValid()) {
		CachedAbilitySystemComponent = UGCFAbilitySystemFunctionLibrary::GetPawnAbilitySystemComponent(Cast<APawn>(GetOwner()));
	}

	if (UAbilitySystemComponent* ASC = CachedAbilitySystemComponent.Get()) {
		if (ASC->HasMatchingGameplayTag(GCFGameplayTags::Gameplay_Movement_Stop)) {
			return 0;
		}
	}
	return Super::GetMaxSpeed();
}