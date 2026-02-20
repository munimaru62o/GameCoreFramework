// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#pragma once

#include "Actor/Character/GCFCharacter.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "GCFCharacterWithAbilities.generated.h"


#define UE_API GAMECOREFRAMEWORK_API

class UGCFAbilitySystemComponent;
class UGCFPawnInputBridgeComponent;

/**
 * @brief Base character class that self-manages its Ability System Component (ASC).
 *
 * Unlike standard player characters that rely on PlayerState for ASC persistence,
 * this class owns the ASC directly.
 *
 * [Use Cases]
 * - AI Characters (Mobs, Bosses, NPCs)
 * - Summons / Minions
 * - Vehicles or dynamic objects with health/abilities
 */
UCLASS()
class AGCFCharacterWithAbilities : public AGCFCharacter, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	AGCFCharacterWithAbilities(const FObjectInitializer& ObjectInitializer);

	//~IAbilitySystemInterface interface
	UFUNCTION(BlueprintCallable, Category = "GCF|Character")
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface interface

	//~IGameplayTagAssetInterface interface
	UE_API virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	UE_API virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	UE_API virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface interface

protected:
	UE_API virtual void OnAbilitySystemInitialized();
	UE_API virtual void OnAbilitySystemUninitialized();

	UE_API void InitializeGameplayTags();

	UE_API void UninitAndDestroy();

	UE_API void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled);
	UE_API virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	UE_API virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	UE_API virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	UE_API virtual bool CanJumpInternal_Implementation() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Character")
	TObjectPtr<UGCFAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Character")
	TObjectPtr<UGCFPawnInputBridgeComponent> PawnInputBridgeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFHealthComponent> HealthComponent;

	// Health attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class UGCFHealthAttributeSet> HealthSet;
	// Combat attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class UGCFCombatAttributeSet> CombatSet;
};

#undef UE_API
