// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/AttributeSet/GCFHealthAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffectTypes.h"
#include "Net/UnrealNetwork.h"


UGCFHealthAttributeSet::UGCFHealthAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
}


void UGCFHealthAttributeSet::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGCFHealthAttributeSet, Health);
	DOREPLIFETIME(UGCFHealthAttributeSet, MaxHealth);
}


void UGCFHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void UGCFHealthAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}


void UGCFHealthAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute()) {
		if (GetHealth() > NewValue) {
			// Adjust health so that it does not exceed maxhealth.
			UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
			check(ASC);
			ASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
		}
	}
}


void UGCFHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	const float OldHealthValue = GetHealth();
	const float MaxHealthValue = GetMaxHealth();

	if (Data.EvaluatedData.Attribute == GetDamageAttribute()) {
		SetHealth(FMath::Clamp(GetHealth() - GetDamage(), 0.0f, GetMaxHealth()));
		SetDamage(0.0f);
	}

	const float CurrentHealthValue = GetHealth();
	if (CurrentHealthValue != OldHealthValue) {
		OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, OldHealthValue, CurrentHealthValue);
	}

	if (CurrentHealthValue <= 0.0f) {
		OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, OldHealthValue, CurrentHealthValue);
	}
}


void UGCFHealthAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute()) {
		// Do not allow health to go negative or above max health.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	} else if (Attribute == GetMaxHealthAttribute()) {
		// Max health must be greater  than 1.0f.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}