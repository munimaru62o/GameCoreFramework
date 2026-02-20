// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GCFLocomotionHandler.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

UINTERFACE(MinimalAPI, Blueprintable)
class UGCFLocomotionHandler : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief Interface to abstract Pawn movement operations (Walking, Driving, Flying, etc.).
 * The ControlComponent passes raw input via this interface, delegating the actual
 * movement logic and calculations to the specific Pawn implementation.
 */
class UE_API IGCFLocomotionHandler
{
	GENERATED_BODY()

public:
	/**
	 * @brief Handles movement input commands.
	 * @param InputValue The input vector. (X: Forward/Throttle, Y: Right/Steer)
	 * @param MovementRotation The reference rotation calculated from the active Rotation Policy.
	 * (e.g., Camera Yaw for TPS, or World Zero for Fixed/TopDown view).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Locomotion")
	void HandleMoveInput(const FVector2D& InputValue, const FRotator& MovementRotation);

	/**
	 * @brief Handles vertical movement input.
	 * Primarily used for altitude control in flying vehicles (Helicopters, Drones),
	 * distinct from standard character jumping.
	 * @param Value The magnitude of the upward/downward input.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Locomotion")
	void HandleMoveUpInput(float Value);
};

#undef UE_API
