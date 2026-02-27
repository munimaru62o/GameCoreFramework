// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Actor/Avatar/GCFAvatarPawn.h"
#include "GCFHumanoid.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

/**
 * @brief The base humanoid class used by this project, deriving from AvatarPawn.
 * 
 * This class specializes the AvatarPawn by assuming the presence of a Capsule Collision
 * and implementing humanoid-specific mechanics such as Crouching (Stance changes).
 * 
 * It provides direct API methods for Control Components (to push input intents)
 * and Mover Input Producers (to pull cached intents), ensuring strict type safety
 * and optimal performance through direct casting.
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "A humanoid pawn class supporting capsule-based stance changes like crouching."))
class AGCFHumanoid : public AGCFAvatarPawn
{
	GENERATED_BODY()

public:
	UE_API AGCFHumanoid(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	class UCapsuleComponent* GetCapsuleComponent() const;

	//~APawn interface
	/** Overridden to adjust eye height dynamically based on crouch stance. */
	UE_API virtual FVector GetPawnViewLocation() const override;
	//~End of APawn interface

	//~IGCFLocomotionInputHandler Interface
	UE_API virtual void HandleCrouchInput_Implementation(bool bIsPressed) override;
	//~End of IGCFLocomotionInputHandler Interface

	//~IGCFLocomotionInputProvider Interface
	UE_API virtual bool GetWantsToCrouch_Implementation() const override;
	//~End of IGCFLocomotionInputProvider Interface

protected:
	// --- Cached Input Intents for Humanoid ---
	/** 
	 * Indicates whether the player currently intends to crouch.
	 * Note: This is purely an input intent, NOT the actual physical state.
	 * To check the actual state, query the MoverComponent for the "Mover.IsCrouching" tag.
	 */
	bool bWantsToCrouch = false;

	/** Tracks the physical state of the crouch button to prevent continuous toggling while held. */
	bool bIsCrouchButtonPressed = false;
};

#undef UE_API