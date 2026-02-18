// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/Predicate/GCFFeaturePredicate.h"
#include "GCFShared.h"
#include "Components/GameFrameworkComponentManager.h"


bool FGCFFeaturePredicate::Evaluate() const
{
	// 1. Ensure the actor is still alive
	if (!Actor.IsValid()) {
		return false;
	}

	// 2. Ensure the manager system is available
	if (UGameFrameworkComponentManager* Manager = GFCM.Get()) {
		// Query the current state tag for the requested feature
		const FGameplayTag Tag = Manager->GetInitStateForFeature(Actor.Get(), Feature);

		// Evaluate: Is it exactly in the 'Ready' state?
		if (Tag == GCFGameplayTags::FeatureState_Ready) {
			return true;
		}
	}
	return false;
}