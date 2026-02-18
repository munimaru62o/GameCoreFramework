// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GCFHealthAttributeSet.generated.h"

#define UE_API GAMECOREFRAMEWORK_API 

struct FGameplayEffectSpec;
class AActor;


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


/**
 * Delegate used to broadcast attribute events, some of these parameters may be null on clients:
 * @param EffectInstigator	The original instigating actor for this event
 * @param EffectCauser		The physical actor that caused the change
 * @param EffectSpec		The full effect spec for this change
 * @param EffectMagnitude	The raw magnitude, this is before clamping
 * @param OldValue			The value of the attribute before it was changed
 * @param NewValue			The value after it was changed
*/
DECLARE_MULTICAST_DELEGATE_SixParams(FAttributeEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec* /*EffectSpec*/, float /*EffectMagnitude*/, float /*OldValue*/, float /*NewValue*/);



/**
 *
 */
UCLASS(MinimalAPI)
class UGCFHealthAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UGCFHealthAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

	// Delegate when health changes due to damage/healing, some information may be missing on the client
	mutable FAttributeEvent OnHealthChanged;

	// Delegate when max health changes
	mutable FAttributeEvent OnMaxHealthChanged;

	// Delegate to broadcast when the health attribute reaches zero
	mutable FAttributeEvent OnOutOfHealth;

	ATTRIBUTE_ACCESSORS(UGCFHealthAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UGCFHealthAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UGCFHealthAttributeSet, Damage);
	ATTRIBUTE_ACCESSORS(UGCFHealthAttributeSet, Heal);

protected:
	// Current health
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, meta = (HideFromModifiers))
	FGameplayAttributeData Health;

	// Upper limit for health value
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	FGameplayAttributeData MaxHealth;

	// Incoming healing. This is mapped directly to -Health
	UPROPERTY(BlueprintReadOnly, Category = "GCF|Health", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Damage;

	// Incoming healing. This is mapped directly to +Health
	UPROPERTY(BlueprintReadOnly, Category = "GCF|Health", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Heal;
};


#undef UE_API
#undef ATTRIBUTE_ACCESSORS