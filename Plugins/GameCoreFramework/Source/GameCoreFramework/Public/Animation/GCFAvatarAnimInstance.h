// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayEffectTypes.h"
#include "GCFAvatarAnimInstance.generated.h"

class APawn;
class UMoverComponent;
class UAbilitySystemComponent;

/**
 * @brief Base AnimInstance for Avatar Pawns using the Mover plugin.
 *
 * Replaces the traditional Character-based AnimInstance.
 * Optimized for UE5's Fast Path by separating data gathering (Game Thread)
 * from heavy mathematical calculations (Worker Thread).
 */
UCLASS(Config = Game)
class GAMECOREFRAMEWORK_API UGCFAvatarAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UGCFAvatarAnimInstance(const FObjectInitializer& ObjectInitializer);

	// Called once at the beginning. Good for caching component pointers.
	virtual void NativeInitializeAnimation() override;

	// Executed on the GAME THREAD. Used ONLY to safely gather data from Actors/Components.
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// Executed on a WORKER THREAD. Used for math and logic using the cached data.
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	/** 
	 * Checks whether the character currently has acceleration intent.
	 * Handles both locally controlled (InputCmd) and simulated proxy (SyncState) scenarios cleanly.
	 */
	bool HasAcceleration() const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "GCF|References")
	TObjectPtr<APawn> OwningPawn;

	UPROPERTY(BlueprintReadOnly, Category = "GCF|References")
	TObjectPtr<UMoverComponent> MoverComponent;

	// --- Cached Raw Data (Gathered on Game Thread) ---
	/** Cached rotation of the pawn to be safely used in the worker thread. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Locomotion")
	FRotator CachedActorRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Locomotion")
	FVector Velocity;

	/** True if the pawn has any input acceleration (i.e., player is pressing keys) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Locomotion")
	bool bHasAcceleration;

	/** True if the pawn is in the air (Falling/Jumping) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Locomotion")
	bool bIsFalling;

	/** True if the pawn is crouching (Determined by GameplayTag) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Locomotion")
	bool bIsCrouched;

	/** Current Movement Mode name from Mover (e.g., 'Walking', 'Falling', 'Flying') */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Locomotion")
	FName CurrentMovementMode;

	// --- Calculated Data (Computed on Worker Thread) ---
	/** Speed of the pawn (Horizontal only, ignoring Z) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Locomotion")
	float GroundSpeed;

	/** Vertical velocity (Z) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Locomotion")
	float VerticalVelocity;

	/** Direction of movement relative to pawn's rotation (-180.0 to 180.0). Ideal for BlendSpaces. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Locomotion")
	float MovementDirection;

	/** True if the pawn is moving (usually GroundSpeed > Threshold) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Locomotion")
	bool bShouldMove;
};