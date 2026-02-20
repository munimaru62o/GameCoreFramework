// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GCFAbilitySystemFunctionLibrary.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UGCFAbilitySystemComponent;


UCLASS(Abstract, MinimalAPI)
class UGCFAbilitySystemFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Retrieves the Ability System Component (ASC) associated with the PlayerState ("Soul").
	 * * @param TargetActor Can be a Pawn, Controller, or PlayerState.
	 * @return The ASC residing on the PlayerState, or nullptr if not found.
	 */
	UFUNCTION(BlueprintPure, Category = "GCF|AbilitySystem", meta = (DefaultToSelf = "TargetActor"))
	static UAbilitySystemComponent* GetPlayerStateAbilitySystemComponent(AActor* TargetActor);

	template <class T>
	static T* GetPlayerStateAbilitySystemComponent(AActor* TargetActor)
	{
		return Cast<T>(GetPlayerStateAbilitySystemComponent(TargetActor));
	}

	/**
	 * Retrieves the Ability System Component (ASC) attached directly to the Pawn ("Body").
	 * * @param TargetPawn The pawn to check.
	 * @return The ASC residing on the Pawn, ignoring the PlayerState.
	 */
	UFUNCTION(BlueprintPure, Category = "GCF|AbilitySystem", meta = (DefaultToSelf = "TargetPawn"))
	static UAbilitySystemComponent* GetPawnAbilitySystemComponent(APawn* TargetPawn);

	template <class T>
	static T* GetPawnAbilitySystemComponent(APawn* TargetPawn)
	{
		return Cast<T>(GetPawnAbilitySystemComponent(TargetPawn));
	}

	/**
	 * Generic method to retrieve an ASC from an Actor.
	 * Checks if the actor implements IAbilitySystemInterface or has an ASC component component.
	 * * Note: This strictly returns the ASC found on the passed actor. It does NOT traverse up to the PlayerState
	 * unless the passed actor IS the PlayerState.
	 */
	UFUNCTION(BlueprintPure, Category = "GCF|AbilitySystem", meta = (DefaultToSelf = "TargetActor"))
	static UAbilitySystemComponent* ResolveAbilitySystemComponent(AActor* TargetActor);

	template <class T>
	static T* ResolveAbilitySystemComponent(AActor* TargetActor)
	{
		return Cast<T>(ResolveAbilitySystemComponent(TargetActor));
	}

	/**
	 * Determines the correct 'OwnerActor' for GAS initialization (InitAbilityActorInfo).
	 * * [Logic based on Possession]
	 * - Player Possessed Pawn -> Returns PlayerState ("Soul").
	 * - AI or Unpossessed Pawn -> Returns Pawn itself ("Body").
	 * * This supports the architecture where the owner shifts between PlayerState and Pawn dynamically.
	 */
	UFUNCTION(BlueprintPure, Category = "GCF|AbilitySystem", meta = (DefaultToSelf = "TargetActor"))
	static AActor* ResolveAbilitySystemComponentOwner(AActor* TargetActor);

	template <class T>
	static T* ResolveAbilitySystemComponentOwner(AActor* TargetActor)
	{
		return Cast<T>(ResolveAbilitySystemComponentOwner(TargetActor));
	}
};

#undef UE_API