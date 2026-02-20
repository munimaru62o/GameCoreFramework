// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"

/**
 * Interface for a logic composer that calculates an aggregate state bitmask.
 *
 * Design Pattern: Strategy Pattern
 * This interface abstracts the logic of "how" the current state is determined.
 * Consumers (like UGCFPawnReadyStateComponent) delegate the complex evaluation logic
 * to this composer, keeping the component clean and focused on state management.
 */
class IGCFStateComposer
{
public:
	virtual ~IGCFStateComposer() = default;

	/**
	 * Evaluates all underlying conditions and returns the current state as a bitmask.
	 * @return A 32-bit integer representing the combined active flags.
	 */
	virtual uint32 Compute() const = 0;
};