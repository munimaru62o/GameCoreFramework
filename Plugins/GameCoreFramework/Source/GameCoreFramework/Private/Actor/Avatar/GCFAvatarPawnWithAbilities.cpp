// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Actor/Avatar/GCFAvatarPawnWithAbilities.h"

#include "System/Lifecycle/GCFPawnExtensionComponent.h"
#include "AbilitySystem/GCFAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/GCFHealthAttributeSet.h"
#include "AbilitySystem/AttributeSet/GCFCombatAttributeSet.h"
#include "Actor/GCFHealthComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFAvatarPawnWithAbilities)


AGCFAvatarPawnWithAbilities::AGCFAvatarPawnWithAbilities(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UGCFAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// Set to 'Mixed' replication mode as this Avatar is intended to be possessed by Players.
	// This allows for client-side prediction of abilities and gameplay effects.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthComponent = CreateDefaultSubobject<UGCFHealthComponent>(TEXT("HealthComponent"));

	// Bind to death events to handle custom Avatar death animations/physics if needed.
	HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);

	// Create AttributeSets.
	HealthSet = CreateDefaultSubobject<UGCFHealthAttributeSet>(TEXT("HealthSet"));
	CombatSet = CreateDefaultSubobject<UGCFCombatAttributeSet>(TEXT("CombatSet"));

	// [Important] Hook into the PawnExtensionComponent lifecycle events.
	// We need to initialize the HealthComponent when the ASC is ready.
	// The PawnExtensionComponent is a required component guaranteed to be created in the base AGCFPawn.
	if (PawnExtensionComponent) {
		PawnExtensionComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
		PawnExtensionComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));
	}
}


UAbilitySystemComponent* AGCFAvatarPawnWithAbilities::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


void AGCFAvatarPawnWithAbilities::OnAbilitySystemInitialized()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	check(ASC);

	// Link HealthComponent to ASC for attribute change listening.
	// Without this, OnDeathStarted will never fire when Health reaches 0.
	HealthComponent->InitializeWithAbilitySystem(ASC);
}


void AGCFAvatarPawnWithAbilities::OnAbilitySystemUninitialized()
{
	HealthComponent->UninitializeFromAbilitySystem();
}


void AGCFAvatarPawnWithAbilities::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	// Clear the output container first to ensure a clean state.
	TagContainer.Reset();

	// 1. Extract tags from the parent class using a temporary container.
	// This safely bypasses any internal Reset() calls in the base implementation
	// that could wipe out our target container.
	FGameplayTagContainer ParentTags;
	Super::GetOwnedGameplayTags(ParentTags);
	TagContainer.AppendTags(ParentTags);

	// 2. Extract and append tags from the Ability System Component safely.
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent()) {
		FGameplayTagContainer ASCTags;
		ASC->GetOwnedGameplayTags(ASCTags);
		TagContainer.AppendTags(ASCTags);
	}
}


bool AGCFAvatarPawnWithAbilities::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	// Check the parent class first.
	if (Super::HasMatchingGameplayTag(TagToCheck)) {
		return true;
	}

	// Fallback to checking the Ability System Component.
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent()) {
		return ASC->HasMatchingGameplayTag(TagToCheck);
	}
	return false;
}


bool AGCFAvatarPawnWithAbilities::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	// [IMPORTANT] To prevent the "split tags" issue where the required tags are distributed 
	// between the parent class and the ASC, we must aggregate all owned tags before evaluation.
	FGameplayTagContainer AllTags;
	GetOwnedGameplayTags(AllTags);

	return AllTags.HasAll(TagContainer);
}


bool AGCFAvatarPawnWithAbilities::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	// Check if the parent class has any of the matching tags.
	if (Super::HasAnyMatchingGameplayTags(TagContainer)) {
		return true;
	}

	// Fallback to checking the Ability System Component.
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent()) {
		return ASC->HasAnyMatchingGameplayTags(TagContainer);
	}
	return false;
}