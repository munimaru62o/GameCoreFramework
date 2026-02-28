// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Actor/Avatar/GCFAvatarPawn.h"
#include "GCFHumanoid.generated.h"

/**
 * @brief The base humanoid class used by this project, deriving from AvatarPawn.
 * 
 * This class specializes the AvatarPawn by assuming the presence of a Capsule Collision
 * and implementing humanoid-specific mechanics such as Crouching (Stance changes).
 * 
 * It extends the interface-driven architecture by providing implementations for
 * humanoid-specific discrete actions (e.g., Crouch), securely exposing these intents
 * to Control Components and Mover Input Producers without concrete coupling.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "A humanoid pawn class supporting capsule-based stance changes like crouching."))
class GAMECOREFRAMEWORK_API AGCFHumanoid : public AGCFAvatarPawn
{
	GENERATED_BODY()

public:
	AGCFHumanoid(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	class UCapsuleComponent* GetCapsuleComponent() const;

	//~APawn interface
	/** Overridden to adjust eye height dynamically based on crouch stance. */
	virtual FVector GetPawnViewLocation() const override;
	//~End of APawn interface

	//~IGCFLocomotionInputHandler Interface
	virtual void HandleCrouchInput_Implementation(bool bIsPressed) override;
	//~End of IGCFLocomotionInputHandler Interface

	//~IGCFLocomotionInputProvider Interface
	virtual bool GetWantsToCrouch_Implementation() const override;
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