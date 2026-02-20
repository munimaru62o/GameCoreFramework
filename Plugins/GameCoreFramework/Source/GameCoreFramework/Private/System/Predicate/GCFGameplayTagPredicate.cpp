// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/Predicate/GCFGameplayTagPredicate.h"
#include "GCFShared.h"
#include "Components/GameFrameworkComponentManager.h"


bool FGCFGameplayTagPredicate::Evaluate() const
{
	if (!Actor.IsValid()) {
		return false;
	}

	if (GFCM.IsValid()) {
		const FGameplayTag CurrentTag = GFCM->GetInitStateForFeature(Actor.Get(), Feature);
		// Explicitly check for an exact match to avoid ambiguity with tag hierarchy.
		// Initialization states are distinct milestones, so "Child" tags should not satisfy "Parent" requirements here.
		if (CurrentTag.MatchesTagExact(Tag)) {
			return true;
		}
	}
	return false;
}