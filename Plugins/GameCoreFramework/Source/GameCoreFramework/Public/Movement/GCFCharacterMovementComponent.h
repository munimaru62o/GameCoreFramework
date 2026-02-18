// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 62o. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Movement/GCFMovementConfigReceiver.h"
#include "GCFCharacterMovementComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UAbilitySystemComponent;

/**
 * @brief Struct to cache ground trace information.
 *
 * Prevents multiple line traces per frame when various systems (IK, Animation, Movement)
 * all need to know "what is under the character's feet".
 */
USTRUCT(BlueprintType)
struct FGCFCharacterGroundInfo
{
	GENERATED_BODY()

	FGCFCharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	uint64 LastUpdateFrame;

	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};

/**
 * @brief GCF-extended CharacterMovementComponent.
 *
 * [Features]
 * - Implements IGCFMovementConfigReceiver for data-driven configuration.
 * - Efficient Ground Tracing cache (GetGroundInfo).
 * - Ability System integration (Tag-based movement blocking).
 * - Based on Lyra's robust movement implementation.
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = Pawn, Blueprintable, Meta = (BlueprintSpawnableComponent))
class UGCFCharacterMovementComponent : public UCharacterMovementComponent, public IGCFMovementConfigReceiver
{
	GENERATED_BODY()

public:
	UE_API UGCFCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	//~IGCFMovementConfigReceiver interface
	UE_API virtual void ApplyMovementConfig_Implementation(const UGCFMovementConfig* Config) override;
	//~End of IGCFMovementConfigReceiver interface

	//~UActorComponent interface
	UE_API virtual void BeginPlay() override; // Added for caching
	//~End of UActorComponent interface

	/**
	 * Returns the current ground info.
	 * Updates the trace only if the frame counter has changed since the last call.
	 */
	UFUNCTION(BlueprintCallable, Category = "GCF|CharacterMovement")
	const FGCFCharacterGroundInfo& GetGroundInfo();

	/** Allows external systems to force a replicated acceleration (used for prediction corrections). */
	UE_API void SetReplicatedAcceleration(const FVector& InAcceleration);

protected:
	virtual void SimulateMovement(float DeltaTime) override;
	virtual bool CanAttemptJump() const override;

	//~UMovementComponent interface
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	virtual float GetMaxSpeed() const override;
	//~End of UMovementComponent interface

private:
	/** Cached ground information. */
	FGCFCharacterGroundInfo CachedGroundInfo;

	/** True if we are using a forced acceleration for replication. */
	bool bHasReplicatedAcceleration = false;

	/** 
	 * Cached pointer to the Ability System Component.
	 * Marked 'mutable' to allow lazy initialization inside const functions (GetMaxSpeed).
	 */
	mutable TWeakObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;
};

#undef UE_API