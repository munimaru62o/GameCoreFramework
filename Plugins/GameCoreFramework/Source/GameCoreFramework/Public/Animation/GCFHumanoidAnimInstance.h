// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayEffectTypes.h"
#include "GCFHumanoidAnimInstance.generated.h"

class APawn;
class UMoverComponent;
class UAbilitySystemComponent;

/**
 * @brief Base AnimInstance for Humanoid Pawns using Mover plugin.
 * 
 * Replaces the traditional Character-based AnimInstance.
 * It fetches movement data directly from MoverComponent and GameplayTags.
 */
UCLASS(Config = Game)
class GAMECOREFRAMEWORK_API UGCFHumanoidAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UGCFHumanoidAnimInstance(const FObjectInitializer& ObjectInitializer);

	// Like BeginPlay for AnimInstances. Good for caching pointers.
	virtual void NativeInitializeAnimation() override;

	// Like Tick for AnimInstances. Used to update variables every frame.
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "GCF|References")
	TObjectPtr<APawn> OwningPawn;

	UPROPERTY(BlueprintReadOnly, Category = "GCF|References")
	TObjectPtr<UMoverComponent> MoverComponent;

	// --- Animation State Variables (Read by AnimGraph) ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Locomotion")
	FVector Velocity;

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
};