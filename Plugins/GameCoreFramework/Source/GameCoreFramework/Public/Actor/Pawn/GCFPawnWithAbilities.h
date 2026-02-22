// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#pragma once

#include "Actor/Pawn/GCFPawn.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayCueInterface.h"
#include "GCFPawnWithAbilities.generated.h"


#define UE_API GAMECOREFRAMEWORK_API

class UGCFAbilitySystemComponent;
class UGCFHealthComponent;

/**
 * @brief Base Pawn class that self-manages its Ability System Component (ASC).
 *
 * Designed for Non-Humanoid actors that require GAS features but do not need
 * the complexity of ACharacter (Capsule/Walking Movement).
 *
 * [Use Cases]
 * - Drones / Flying Vehicles
 * - Turrets / Static Defenses
 * - Destructible Objects with Health
 * - Simple AI Minions
 */
UCLASS()
class AGCFPawnWithAbilities : public AGCFPawn, public IAbilitySystemInterface, public IGameplayTagAssetInterface, public IGameplayCueInterface
{
	GENERATED_BODY()

public:
	AGCFPawnWithAbilities(const FObjectInitializer& ObjectInitializer);

	//~IAbilitySystemInterface interface
	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface interface

	//~IGameplayTagAssetInterface interface
	UE_API virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	UE_API virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface interface

protected:
	/** Called when the ASC is initialized (Registered to the Extension Component). */
	virtual void OnAbilitySystemInitialized();

	/** Called when the ASC is uninitialized. */
	virtual void OnAbilitySystemUninitialized();

protected:
	/** The Ability System Component owned by this pawn. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn")
	TObjectPtr<UGCFAbilitySystemComponent> AbilitySystemComponent;

	/** Handles health, death, and damage logic. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFHealthComponent> HealthComponent;

	/** Health Attribute Set (HP, MaxHP, Healing). */
	UPROPERTY()
	TObjectPtr<const class UGCFHealthAttributeSet> HealthSet;

	/** Combat Attribute Set (Damage, Defense, etc.). */
	UPROPERTY()
	TObjectPtr<const class UGCFCombatAttributeSet> CombatSet;
};

#undef UE_API
