// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Actor/Avatar/GCFAvatarPawn.h"
#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GCFAvatarPawnWithAbilities.generated.h"

class UGCFAbilitySystemComponent;
class UGCFHealthComponent;

/**
 * @brief Avatar Pawn class that self-manages its own Ability System Component (ASC).
 *
 * This class combines the Mover-driven physical shell (AGCFAvatarPawn) with the
 * Gameplay Ability System (GAS). It is intended for complex characters that need
 * both advanced locomotion (Mover) and combat/ability features (Health, Attributes).
 *
 * [Use Cases]
 * - AI Humanoid Enemies / Bosses
 * - Player Characters in a Single-ASC setup
 * - The "Body" part in a Dual-ASC setup (handling physical abilities like Jump/Dash)
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base avatar pawn with GAS capabilities."))
class GAMECOREFRAMEWORK_API AGCFAvatarPawnWithAbilities : public AGCFAvatarPawn, public IAbilitySystemInterface, public IGameplayCueInterface
{
	GENERATED_BODY()

public:
	AGCFAvatarPawnWithAbilities(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~IAbilitySystemInterface interface
	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface interface

	//~IGameplayTagAssetInterface interface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface interface

protected:
	/** Called when the ASC is initialized (Registered to the Extension Component). */
	virtual void OnAbilitySystemInitialized();

	/** Called when the ASC is uninitialized. */
	virtual void OnAbilitySystemUninitialized();

protected:
	/** The Ability System Component owned by this pawn. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Abilities")
	TObjectPtr<UGCFAbilitySystemComponent> AbilitySystemComponent;

	/** Handles health, death, and damage logic. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Abilities", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFHealthComponent> HealthComponent;

	/** Health Attribute Set (HP, MaxHP, Healing). */
	UPROPERTY()
	TObjectPtr<const class UGCFHealthAttributeSet> HealthSet;

	/** Combat Attribute Set (Damage, Defense, etc.). */
	UPROPERTY()
	TObjectPtr<const class UGCFCombatAttributeSet> CombatSet;
};