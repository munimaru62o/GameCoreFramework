// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "System/Composer/GCFStateComposer.h"
#include "System/Predicate/GCFStatePredicate.h"

#include "Containers/Array.h"
#include "Templates/SharedPointer.h"

/**
 * A concrete implementation of the State Composer that aggregates multiple predicates.
 *
 * Mechanism:
 * It maintains a list of {Bit, Predicate} pairs.
 * During computation, it iterates through all registered entries.
 * If a Predicate evaluates to true, the corresponding Bit is OR'ed into the result.
 *
 * [Benefit]
 * This allows for highly flexible and dynamic state definitions.
 * You can mix and match different types of predicates (Feature-based, GameplayTag-based, etc.)
 * without modifying the core logic of the component.
 */
class FGCFGenericStateComposer final : public IGCFStateComposer
{
public:
	/** Internal mapping entry between a state bit and its condition. */
	struct FEntry
	{
		uint32 Bit;
		TSharedRef<IGCFStatePredicate> Predicate;
	};

	/**
	 * Registers a new condition for a specific state bit.
	 *
	 * @param Bit       The target bit flag (e.g., EGCFPawnReadyState::Ability).
	 * @param Predicate The condition object that determines if this bit should be set.
	 */
	void Add(uint32 Bit, TSharedRef<IGCFStatePredicate> Predicate);

	/**
	 * Iterates through all entries and constructs the bitmask.
	 */
	virtual uint32 Compute() const override;

private:
	/** List of logic rules to evaluate. */
	TArray<FEntry> Entries;
};
