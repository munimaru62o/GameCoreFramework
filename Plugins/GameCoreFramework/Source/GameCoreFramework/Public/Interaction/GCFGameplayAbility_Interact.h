// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "AbilitySystem/GCFGameplayAbility.h"
#include "GCFGameplayAbility_Interact.generated.h"

/**
 * @brief Base Gameplay Ability for Interactions.
 *
 * [Workflow]
 * 1. Activated via Input Tag (e.g., 'Input.Interact').
 * 2. Retrieves the current target from the Controller's UGCFInteractionComponent (Client side).
 * 3. Validates the target (Range, Interface checks).
 * 4. Syncs target to Server via RPC (if Client) or executes directly (if Host/AI).
 * 5. Executes the interaction logic (TriggerTargetInteraction).
 */
UCLASS(Abstract)
class GAMECOREFRAMEWORK_API UGCFGameplayAbility_Interact : public UGCFGameplayAbility
{
	GENERATED_BODY()

public:
	UGCFGameplayAbility_Interact();

protected:
	/**
	 * Called when the ability is activated.
	 * Handles target resolution, local prediction/commit, and routing execution to the Server.
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** Server RPC to receive the target identified by the client. */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ProcessInteraction(AActor* TargetActor);

	/** Executes the actual interaction logic (called on Server). */
	void ProcessInteractionLogic();

	/**
	 * Calls the 'OnInteract' interface function on the target actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "GCF|Interaction")
	void TriggerTargetInteraction();

	/**
	 * Checks if the target is valid, implements the interface, and is within range.
	 */
	UFUNCTION(BlueprintCallable, Category = "GCF|Interaction")
	bool ValidateInteractionTarget() const;

protected:
	/** The actor we intend to interact with. Resolved during activation. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<AActor> InteractionTarget;
};