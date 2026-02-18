// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/AttributeSet/GCFAttributeSet.h"
#include "AbilitySystem/GCFAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFAttributeSet)

class UWorld;


UGCFAttributeSet::UGCFAttributeSet()
{}

UWorld* UGCFAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UGCFAbilitySystemComponent* UGCFAttributeSet::GetGCFAbilitySystemComponent() const
{
	return Cast<UGCFAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}

