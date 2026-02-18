// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Actor/Character/GCFCharacterWithAbilities.h"
#include "AbilitySystem/GCFAbilitySystemComponent.h"
#include "Input/GCFPawnInputBridgeComponent.h"
#include "Actor/GCFHealthComponent.h"
#include "System/Lifecycle/GCFPawnExtensionComponent.h"
#include "Movement/GCFCharacterMovementComponent.h"
#include "AbilitySystem/AttributeSet/GCFHealthAttributeSet.h"
#include "AbilitySystem/AttributeSet/GCFCombatAttributeSet.h"


AGCFCharacterWithAbilities::AGCFCharacterWithAbilities(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create ASC locally.
	// For AI/NPCs, Mixed or Minimal replication mode is efficient.
	AbilitySystemComponent = CreateDefaultSubobject<UGCFAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	PawnInputBridgeComponent = CreateDefaultSubobject<UGCFPawnInputBridgeComponent>(TEXT("PawnInputBridgeComponent"));

	HealthComponent = CreateDefaultSubobject<UGCFHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);

	// Create AttributeSets. These are automatically detected by the ASC during InitializeComponent,
	// but we hold a hard reference here to ensure they persist until then.
	HealthSet = CreateDefaultSubobject<UGCFHealthAttributeSet>(TEXT("HealthSet"));
	CombatSet = CreateDefaultSubobject<UGCFCombatAttributeSet>(TEXT("CombatSet"));

	// Hook into the PawnExtensionComponent lifecycle events
	if (PawnExtComponent) {
		PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
		PawnExtComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));
	}
}


UAbilitySystemComponent* AGCFCharacterWithAbilities::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


void AGCFCharacterWithAbilities::OnAbilitySystemInitialized()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	check(ASC);

	HealthComponent->InitializeWithAbilitySystem(ASC);
	InitializeGameplayTags();
}

void AGCFCharacterWithAbilities::OnAbilitySystemUninitialized()
{
	HealthComponent->UninitializeFromAbilitySystem();
}


void AGCFCharacterWithAbilities::InitializeGameplayTags()
{
	// Clear tags that may be lingering on the ability system from the previous pawn.
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent()) {
		for (const TPair<uint8, FGameplayTag>& TagMapping : GCFGameplayTags::MovementModeTagMap) {
			if (TagMapping.Value.IsValid()) {
				ASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}
		}

		for (const TPair<uint8, FGameplayTag>& TagMapping : GCFGameplayTags::CustomMovementModeTagMap) {
			if (TagMapping.Value.IsValid()) {
				ASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}

			UCharacterMovementComponent* GCFMoveComp = CastChecked<UCharacterMovementComponent>(GetCharacterMovement());
			SetMovementModeTag(GCFMoveComp->MovementMode, GCFMoveComp->CustomMovementMode, true);
		}
	}
}


void AGCFCharacterWithAbilities::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent()) {
		ASC->GetOwnedGameplayTags(TagContainer);
	}
}

bool AGCFCharacterWithAbilities::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent()) {
		return ASC->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

bool AGCFCharacterWithAbilities::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent()) {
		return ASC->HasAllMatchingGameplayTags(TagContainer);
	}

	return false;
}

bool AGCFCharacterWithAbilities::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent()) {
		return ASC->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
}


void AGCFCharacterWithAbilities::UninitAndDestroy()
{
	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent()) {
		if (ASC->GetAvatarActor() == this) {
			PawnExtComponent->UninitializeAbilitySystem();
		}
	}
	Super::UninitAndDestroy();
}


void AGCFCharacterWithAbilities::SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent()) {
		const FGameplayTag* MovementModeTag = nullptr;
		if (MovementMode == MOVE_Custom) {
			MovementModeTag = GCFGameplayTags::CustomMovementModeTagMap.Find(CustomMovementMode);
		} else {
			MovementModeTag = GCFGameplayTags::MovementModeTagMap.Find(MovementMode);
		}

		if (MovementModeTag && MovementModeTag->IsValid()) {
			ASC->SetLooseGameplayTagCount(*MovementModeTag, (bTagEnabled ? 1 : 0));
		}
	}
}


void AGCFCharacterWithAbilities::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	UCharacterMovementComponent* MoveComponent = CastChecked<UCharacterMovementComponent>(GetCharacterMovement());

	SetMovementModeTag(PrevMovementMode, PreviousCustomMode, false);
	SetMovementModeTag(MoveComponent->MovementMode, MoveComponent->CustomMovementMode, true);
}


void AGCFCharacterWithAbilities::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent()) {
		ASC->SetLooseGameplayTagCount(GCFGameplayTags::Status_Crouching, 1);
	}
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}


void AGCFCharacterWithAbilities::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent()) {
		ASC->SetLooseGameplayTagCount(GCFGameplayTags::Status_Crouching, 0);
	}

	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}


bool AGCFCharacterWithAbilities::CanJumpInternal_Implementation() const
{
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent()) {
		// Block jump if the 'JumpBlocked' tag is present (e.g., Stunned, Rooted).
		if (ASC->HasMatchingGameplayTag(GCFGameplayTags::State_JumpBlocked)) {
			return false;
		}
	}

	// same as ACharacter's implementation but without the crouch check
	return Super::CanJumpInternal_Implementation();
}
