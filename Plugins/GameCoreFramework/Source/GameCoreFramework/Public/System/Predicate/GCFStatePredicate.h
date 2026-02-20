// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"

/**
 * Interface representing a single, atomic condition (Predicate) for state evaluation.
 *
 * Design Pattern: Strategy / Command Pattern
 * By encapsulating a condition into an object, we can treat diverse checks
 * (e.g., "Is Actor Alive?", "Is Feature Active?", "Does Tag Exist?") uniformly.
 *
 * This allows the StateComposer to iterate over a collection of these predicates
 * without knowing the implementation details of each check.
 */
class IGCFStatePredicate
{
public:
	virtual ~IGCFStatePredicate() = default;

	/**
	 * Evaluates the specific condition.
	 * @return true if the condition is met, false otherwise.
	 */
	virtual bool Evaluate() const = 0;
};