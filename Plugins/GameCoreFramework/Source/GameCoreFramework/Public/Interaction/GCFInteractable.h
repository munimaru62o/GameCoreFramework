// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "GCFInteractable.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class APawn;

UINTERFACE(MinimalAPI, Blueprintable)
class UGCFInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief Interface for objects that can be interacted with by a Pawn/Character.
 *
 * Provides a standard contract for:
 * - Executing interactions (OnInteract)
 * - Visual feedback (Focus/Hover)
 * - Metadata for Gameplay Abilities (Interaction Type/Duration)
 */
class GAMECOREFRAMEWORK_API IGCFInteractable
{
	GENERATED_BODY()

public:
	/**
	 * Triggered when an actor interacts with this object.
	 * @param Interactor The actor initiating the interaction.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Interaction")
	void OnInteract(AActor* Interactor);

	/**
	 * Called when the interactable object gains focus (e.g., crosshair hover, overlap).
	 * Useful for showing UI prompts or outlines.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Interaction")
	void OnBeginFocus();

	/**
	 * Called when the interactable object loses focus.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Interaction")
	void OnEndFocus();

	/**
	 * Returns a GameplayTag identifying the type of interaction.
	 * Used by GAS to determine which Ability to activate (e.g., 'Interaction.Pickup', 'Interaction.Open').
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Interaction")
	FGameplayTag GetInteractionType() const;

	/**
	 * Returns the duration required for the interaction (0.0 for instant).
	 * Useful for hold-to-interact UI progress bars.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Interaction")
	float GetInteractionDuration() const;
};

#undef UE_API