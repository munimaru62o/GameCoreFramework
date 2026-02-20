// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "System/Predicate/GCFStatePredicate.h"

class UGameFrameworkComponentManager;
class AActor;

/**
 * A predicate that evaluates the initialization state of a specific "Feature" on an actor.
 *
 * Context:
 * Modular Gameplay (GameFrameworkComponentManager) manages features via "Init States"
 * (e.g., DataAvailable -> DataInitialized -> GameplayReady).
 *
 * This predicate acts as a bridge, checking if a specific feature (identified by name)
 * has reached the specific 'Ready' state defined by GCFGameplayTags.
 *
 * Use Case:
 * Used by the StateComposer to ensure that dependent systems (e.g., AbilitySystem) are
 * fully initialized before proceeding to the next phase of the pawn's lifecycle.
 */
class FGCFFeaturePredicate final : public IGCFStatePredicate
{
public:
	/**
	 * @param InGFCM    Manager instance to query.
	 * @param InActor   The actor that owns the feature.
	 * @param InFeature The name of the feature to check (e.g., "GameFeature.Ability").
	 */
	FGCFFeaturePredicate(
		UGameFrameworkComponentManager* InGFCM,
		TWeakObjectPtr<AActor> InActor,
		FName InFeature)
		: GFCM(InGFCM)
		, Actor(InActor)
		, Feature(InFeature)
	{}

	virtual bool Evaluate() const override;

private:
	TWeakObjectPtr<UGameFrameworkComponentManager> GFCM;
	TWeakObjectPtr<AActor> Actor;
	FName Feature;
};
