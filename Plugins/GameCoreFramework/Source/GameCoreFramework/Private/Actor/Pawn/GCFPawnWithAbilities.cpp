// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.


#include "Actor/Pawn/GCFPawnWithAbilities.h"
#include "System/Lifecycle/GCFPawnExtensionComponent.h"
#include "AbilitySystem/GCFAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/GCFHealthAttributeSet.h"
#include "AbilitySystem/AttributeSet/GCFCombatAttributeSet.h"
#include "Actor/GCFHealthComponent.h"


AGCFPawnWithAbilities::AGCFPawnWithAbilities(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create ASC locally.
	// For AI/NPCs/Simple Pawns, Minimal replication mode is efficient.
	AbilitySystemComponent = CreateDefaultSubobject<UGCFAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	HealthComponent = CreateDefaultSubobject<UGCFHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);

	// Create AttributeSets.
	HealthSet = CreateDefaultSubobject<UGCFHealthAttributeSet>(TEXT("HealthSet"));
	CombatSet = CreateDefaultSubobject<UGCFCombatAttributeSet>(TEXT("CombatSet"));

	// [Important] Hook into the PawnExtensionComponent lifecycle events.
	// We need to initialize the HealthComponent when the ASC is ready.
	if (PawnExtensionComponent) {
		PawnExtensionComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
		PawnExtensionComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));
	}
}

UAbilitySystemComponent* AGCFPawnWithAbilities::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGCFPawnWithAbilities::OnAbilitySystemInitialized()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	check(ASC);

	// Link HealthComponent to ASC for attribute change listening.
	// Without this, OnDeathStarted will never fire.
	HealthComponent->InitializeWithAbilitySystem(ASC);
}

void AGCFPawnWithAbilities::OnAbilitySystemUninitialized()
{
	HealthComponent->UninitializeFromAbilitySystem();
}


void AGCFPawnWithAbilities::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
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


bool AGCFPawnWithAbilities::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
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


bool AGCFPawnWithAbilities::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	// [IMPORTANT] To prevent the "split tags" issue where the required tags are distributed 
	// between the parent class and the ASC, we must aggregate all owned tags before evaluation.
	FGameplayTagContainer AllTags;
	GetOwnedGameplayTags(AllTags);

	return AllTags.HasAll(TagContainer);
}


bool AGCFPawnWithAbilities::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
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