// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/AttributeSet/GCFCombatAttributeSet.h"
#include "AbilitySystem/AttributeSet/GCFAttributeSet.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFCombatAttributeSet)

class FLifetimeProperty;


UGCFCombatAttributeSet::UGCFCombatAttributeSet()
	: BaseDamage(0.0f)
	, BaseHeal(0.0f)
{}

void UGCFCombatAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGCFCombatAttributeSet, BaseDamage, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGCFCombatAttributeSet, BaseHeal, COND_OwnerOnly, REPNOTIFY_Always);
}

void UGCFCombatAttributeSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGCFCombatAttributeSet, BaseDamage, OldValue);
}

void UGCFCombatAttributeSet::OnRep_BaseHeal(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGCFCombatAttributeSet, BaseHeal, OldValue);
}

