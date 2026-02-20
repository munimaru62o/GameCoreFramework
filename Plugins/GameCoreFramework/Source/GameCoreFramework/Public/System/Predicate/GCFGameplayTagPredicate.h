// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "System/Predicate/GCFStatePredicate.h"


class UGameFrameworkComponentManager;
class AActor;

/**
 * A generic predicate that evaluates if a feature's initialization state matches a specific GameplayTag.
 *
 * [Comparison with FGCFFeaturePredicate]
 * - FGCFFeaturePredicate checks specifically for the final "Ready" state.
 * - This class checks for ANY arbitrary state (e.g., "DataAvailable", "Spawned", "CustomState").
 *
 * Use Case:
 * Useful when the initialization flow has intermediate steps that need to be waited upon
 * before proceeding, or for debugging specific phases of the lifecycle.
 */
class FGCFGameplayTagPredicate final : public IGCFStatePredicate
{
public:
	/**
	 * @param InGFCM    The manager instance.
	 * @param InActor   The actor owning the feature.
	 * @param InFeature The name of the feature to inspect.
	 * @param InTag     The specific state tag to compare against.
	 * (Note: Uses MatchesTagExact, so hierarchy is ignored).
	 */
	FGCFGameplayTagPredicate(
		UGameFrameworkComponentManager* InGFCM,
		TWeakObjectPtr<AActor> InActor,
		FName InFeature,
		FGameplayTag InTag)
		: GFCM(InGFCM)
		, Actor(InActor)
		, Feature(InFeature)
		, Tag(InTag)
	{};

	virtual bool Evaluate() const override;

private:
	TWeakObjectPtr<UGameFrameworkComponentManager> GFCM;
	TWeakObjectPtr<AActor> Actor;
	FName Feature;
	FGameplayTag Tag;
};
