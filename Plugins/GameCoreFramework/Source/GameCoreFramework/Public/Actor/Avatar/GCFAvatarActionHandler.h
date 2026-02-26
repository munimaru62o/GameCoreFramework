// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GCFAvatarActionHandler.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

UINTERFACE(MinimalAPI, Blueprintable)
class UGCFAvatarActionHandler : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief Interface to abstract specialized Avatar actions (Jumping, Crouching, etc.).
 * The ControlComponent passes binary action inputs via this interface, delegating
 * the actual execution and state caching to the specific Avatar implementation.
 */
class UE_API IGCFAvatarActionHandler
{
	GENERATED_BODY()

public:
	/**
	 * @brief Handles jump input commands.
	 * @param bIsPressed True if the jump action is currently being pressed/held, false if released.
	 * Useful for implementing variable jump heights or hold-to-jump mechanics.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Action")
	void HandleJumpInput(bool bIsPressed);

	/**
	 * @brief Handles crouch input commands.
	 * @param bIsPressed True if the crouch action is currently being pressed/held, false if released.
	 * Can be used to implement either toggle-crouch or hold-to-crouch logic within the Pawn.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Action")
	void HandleCrouchInput(bool bIsPressed);
};

#undef UE_API