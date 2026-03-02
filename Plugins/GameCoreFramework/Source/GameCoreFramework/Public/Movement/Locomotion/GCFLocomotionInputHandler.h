// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GCFLocomotionInputHandler.generated.h"


UINTERFACE(MinimalAPI, Blueprintable)
class UGCFLocomotionInputHandler : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief Interface to abstract Pawn movement operations (Walking, Driving, Flying, Jumping, Crouching).
 * The ControlComponent passes raw input via this interface, delegating the actual
 * movement logic, state caching, and calculations to the specific Pawn implementation.
 */
class GAMECOREFRAMEWORK_API IGCFLocomotionInputHandler
{
	GENERATED_BODY()

public:
	/**
	 * @brief Handles movement input commands.
	 * @param InputValue The input vector. (X: Forward/Throttle, Y: Right/Steer)
	 * @param MovementRotation The reference rotation calculated from the active Rotation Policy.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Locomotion")
	void HandleMoveInput(const FVector2D& InputValue, const FRotator& MovementRotation);

	/**
	 * @brief Handles vertical movement input (e.g., altitude control for flying vehicles).
	 * @param Value The magnitude of the upward/downward input.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Locomotion")
	void HandleMoveUpInput(float Value);

	/**
	 * @brief Handles jump input commands.
	 * @param bIsPressed True if the jump action is currently triggered/held, false otherwise.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Locomotion")
	void HandleJumpInput(bool bIsPressed);

	/**
	 * @brief Handles crouch input commands.
	 * @param bIsPressed True if the crouch action is currently triggered, false otherwise.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Locomotion")
	void HandleCrouchInput(bool bIsPressed);
};