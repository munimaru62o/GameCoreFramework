// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "GCFAttributeSet.h"
#include "GCFCombatAttributeSet.generated.h"

class UObject;
struct FFrame;

/**
 * UGCFCombatAttributeSet
 *
 *  Class that defines attributes that are necessary for applying damage or healing.
 *	Attribute examples include: damage, healing, attack power, and shield penetrations.
 */
UCLASS(BlueprintType)
class UGCFCombatAttributeSet : public UGCFAttributeSet
{
	GENERATED_BODY()

public:

	UGCFCombatAttributeSet();

	ATTRIBUTE_ACCESSORS(UGCFCombatAttributeSet, BaseDamage);
	ATTRIBUTE_ACCESSORS(UGCFCombatAttributeSet, BaseHeal);

protected:

	UFUNCTION()
	void OnRep_BaseDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseHeal(const FGameplayAttributeData& OldValue);

private:

	// The base amount of damage to apply in the damage execution.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseDamage, Category = "GCF|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseDamage;

	// The base amount of healing to apply in the heal execution.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseHeal, Category = "GCF|Combat", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseHeal;
};
