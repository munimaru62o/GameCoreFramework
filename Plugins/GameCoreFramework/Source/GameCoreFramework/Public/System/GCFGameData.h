// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "GCFGameData.generated.h"

class UGameplayEffect;
class UObject;

/**
 * UGCFGameData
 *
 *	Non-mutable data asset that contains global game data.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "GCF Game Data", ShortTooltip = "Data asset containing global game data."))
class GAMECOREFRAMEWORK_API UGCFGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UGCFGameData();

	// Returns the loaded game data.
	static const UGCFGameData& Get();

public:

	// Gameplay effect used to apply damage.  Uses SetByCaller for the damage magnitude.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Damage Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> DamageGameplayEffect_SetByCaller;

	// Gameplay effect used to apply healing.  Uses SetByCaller for the healing magnitude.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Heal Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> HealGameplayEffect_SetByCaller;

	// Gameplay effect used to add and remove dynamic tags.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect;
};
