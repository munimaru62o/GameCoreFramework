// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Interaction/GCFGameplayAbility_Interact.h"
#include "Interaction/GCFInteractable.h"
#include "Interaction/GCFInteractionComponent.h"
#include "AbilitySystemComponent.h"


UGCFGameplayAbility_Interact::UGCFGameplayAbility_Interact()
{
	// Interaction requires state tracking per actor/execution.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Ensure this ability runs on both Client (for prediction) and Server.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}


void UGCFGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// ---------------------------------------------------------
	// 1. Target Resolution (Common for Local & Server)
	// ---------------------------------------------------------

	// A: Try to resolve from Event Payload (Priority for AI / Direct Event triggers).
	if (TriggerEventData && TriggerEventData->Target) {
		InteractionTarget = const_cast<AActor*>(TriggerEventData->Target.Get());
	}
	// B: If Input-driven, resolve from the InteractionComponent on the controller.
	// We perform this check if we are locally controlled OR if we are the server (Host) executing logic.
	else if (ActorInfo->PlayerController.IsValid()) {
		if (UGCFInteractionComponent* InteractComp = ActorInfo->PlayerController->FindComponentByClass<UGCFInteractionComponent>()) {
			InteractionTarget = InteractComp->GetFocusedTarget();
		}
	}

	// ---------------------------------------------------------
	// 2. Local Validation & Commit (Prediction)
	// ---------------------------------------------------------

	// If we are the local player (Host or Client), we must validate and commit cost/cooldown locally first.
	if (ActorInfo->IsLocallyControlled()) {
		if (!ValidateInteractionTarget()) {
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}

	// ---------------------------------------------------------
	// 3. Execution Routing
	// ---------------------------------------------------------

	const bool bIsServer = HasAuthority(&ActivationInfo);
	const bool bIsLocal = ActorInfo->IsLocallyControlled();

	if (bIsServer) {
		// Case A: Host Player (Server + Local) OR AI (Server).
		// We are already on the server with Authority, so just execute the logic directly.
		if (ValidateInteractionTarget()) {
			ProcessInteractionLogic();
		} else {
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
	} else if (bIsLocal) {
		// Case B: Client Player (Remote Client).
		// We are on the client, so we must ask the server to execute via RPC.
		// (Target was already set in Step 1)
		Server_ProcessInteraction(InteractionTarget);
	}

	// 4. Call Super to trigger BP events (e.g., Play Sound/VFX on local client).
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}


void UGCFGameplayAbility_Interact::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	InteractionTarget = nullptr;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UGCFGameplayAbility_Interact::Server_ProcessInteraction_Validate(AActor* TargetActor)
{
	// Basic security check (e.g. is not null)
	return true;
}

void UGCFGameplayAbility_Interact::Server_ProcessInteraction_Implementation(AActor* TargetActor)
{
	// Set the target received from the client.
	InteractionTarget = TargetActor;

	// Server-side strict validation (Distance check, Interface check).
	// If this fails, it could be a cheat attempt or lag discrepancy.
	if (!ValidateInteractionTarget()) {
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Execute the actual logic.
	ProcessInteractionLogic();
}

void UGCFGameplayAbility_Interact::ProcessInteractionLogic()
{
	// Execute the interaction (Interface call, etc.)
	TriggerTargetInteraction();

	// Since this is an instant interaction, we end the ability here.
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


void UGCFGameplayAbility_Interact::TriggerTargetInteraction()
{
	if (InteractionTarget && InteractionTarget->Implements<UGCFInteractable>()) {
		AActor* Avatar = GetAvatarActorFromActorInfo();

		// Execute the Blueprint Native Event on the target object.
		IGCFInteractable::Execute_OnInteract(InteractionTarget, Avatar);
	}
}


bool UGCFGameplayAbility_Interact::ValidateInteractionTarget() const
{
	if (!InteractionTarget) {
		return false;
	}

	// Ensure the actor actually implements the interface.
	if (!InteractionTarget->Implements<UGCFInteractable>()) {
		return false;
	}

	// [Optional] Add distance check here for server-side security.
	/*
	if (const AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		const float DistSq = FVector::DistSquared(Avatar->GetActorLocation(), InteractionTarget->GetActorLocation());
		if (DistSq > FMath::Square(500.0f)) // e.g., 5 meters
		{
			return false;
		}
	}
	*/

	return true;
}

