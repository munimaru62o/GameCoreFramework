// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.


#include "AbilitySystem/GCFAbilitySystemFunctionLibrary.h"

#include "GCFShared.h"
#include "AbilitySystem/GCFAbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"


UAbilitySystemComponent* UGCFAbilitySystemFunctionLibrary::GetPlayerStateAbilitySystemComponent(AActor* TargetActor)
{
	if (!IsValid(TargetActor)) {
		return nullptr;
	}

	APlayerState* PS = nullptr;

	// 1. Target is the PlayerState itself.
	if (APlayerState* TargetPS = Cast<APlayerState>(TargetActor)) {
		PS = TargetPS;
	}
	// 2. Target is a Pawn (Traverse: Pawn -> PlayerState).
	else if (APawn* TargetPawn = Cast<APawn>(TargetActor)) {
		PS = TargetPawn->GetPlayerState();
	}
	// 3. Target is a Controller (Traverse: Controller -> PlayerState).
	else if (AController* TargetController = Cast<AController>(TargetActor)) {
		PS = TargetController->PlayerState;
	}

	if (PS) {
		// Prefer Interface call for safety and standard compliance.
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PS)) {
			return ASI->GetAbilitySystemComponent();
		}
		// Fallback: Find component directly.
		return PS->FindComponentByClass<UAbilitySystemComponent>();
	}

	return nullptr;
}


UAbilitySystemComponent* UGCFAbilitySystemFunctionLibrary::GetPawnAbilitySystemComponent(APawn* TargetPawn)
{
	if (IsValid(TargetPawn)) {
		// Strictly look for the ASC on the Pawn ("Body") itself.
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetPawn)) {
			return ASI->GetAbilitySystemComponent();
		}
		return TargetPawn->FindComponentByClass<UAbilitySystemComponent>();
	}
	return nullptr;
}


UAbilitySystemComponent* UGCFAbilitySystemFunctionLibrary::ResolveAbilitySystemComponent(AActor* TargetActor)
{
	if (!IsValid(TargetActor)) {
		return nullptr;
	}

	// Strictly checks the passed actor.
	// Does NOT traverse hierarchy to avoid confusion between PawnASC vs PlayerStateASC.
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetActor)) {
		return ASI->GetAbilitySystemComponent();
	}

	return TargetActor->FindComponentByClass<UAbilitySystemComponent>();
}


AActor* UGCFAbilitySystemFunctionLibrary::ResolveAbilitySystemComponentOwner(AActor* TargetActor)
{
	if (!IsValid(TargetActor)) {
		return nullptr;
	}

	// Case 1: Target is a Pawn
	if (APawn* Pawn = Cast<APawn>(TargetActor)) {
		// If possessed by a Player, the logical owner is the PlayerState ("Soul").
		if (APlayerState* PS = Pawn->GetPlayerState()) {
			return PS;
		}

		// If AI or Unpossessed, the logical owner reverts to the Pawn itself ("Body").
		// This matches the design requirement: "Owner reverts to Self when unpossessed".
		return Pawn;
	}

	// Case 2: Target is a Controller
	if (AController* Controller = Cast<AController>(TargetActor)) {
		if (Controller->PlayerState) {
			return Controller->PlayerState;
		}
		return Controller; // Fallback for AIController without PlayerState
	}

	// Case 3: Target is a PlayerState
	if (Cast<APlayerState>(TargetActor)) {
		return TargetActor;
	}

	// Default: Assume the actor owns itself.
	return TargetActor;
}
