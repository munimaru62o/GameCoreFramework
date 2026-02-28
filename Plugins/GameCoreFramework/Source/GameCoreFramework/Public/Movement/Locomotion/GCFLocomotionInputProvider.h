// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "GCFShared.h"
#include "UObject/Interface.h"
#include "GCFLocomotionInputProvider.generated.h"


UINTERFACE(MinimalAPI)
class UGCFLocomotionInputProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief Interface allowing Input Producers to securely pull cached movement intents
 * from any Pawn, without tightly coupling to specific concrete classes (e.g., AGCFAvatarPawn).
 */
class GAMECOREFRAMEWORK_API IGCFLocomotionInputProvider
{
	GENERATED_BODY()

public:
	// --- Intents (Vectors) ---

	/** 
	 * Returns the world-space direction/magnitude the player intends to move.
	 * This is usually cached from the Controller's input.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Locomotion")
	FVector GetMovementIntent() const;

	/** * Returns the intended forward facing direction (Orientation Intent) of the pawn.
	 * This allows the Pawn to dictate its facing direction (e.g., matching camera, locking on target, or twin-stick aiming).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Locomotion")
	FVector GetOrientationIntent() const;

	// --- Actions (Booleans) ---

	/** Returns true if the jump button is currently being held down. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Locomotion")
	bool GetIsJumpPressed() const;

	/** Returns true ONLY on the exact frame the jump button was initially pressed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Locomotion")
	bool GetIsJumpJustPressed() const;

	/** Called by the Input Producer to clear the JustPressed flag after consuming it. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Locomotion")
	void ConsumeJumpJustPressed();

	/** Returns true if the pawn intends to crouch. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Locomotion")
	bool GetWantsToCrouch() const;
};