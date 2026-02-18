// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "System/Predicate/GCFStatePredicate.h"


class UGameFrameworkComponentManager;
class AActor;

/**
 * A basic predicate that validates the existence (liveness) of an Actor.
 *
 * Usage:
 * Useful as a prerequisite check or for monitoring objects that might be destroyed
 * during gameplay (e.g., a Pawn dying).
 *
 * Safety:
 * Holds a TWeakObjectPtr to ensure it does not prevent garbage collection
 * or cause crashes by accessing dangling pointers.
 */
class FGCFExistPredicate : public IGCFStatePredicate
{
public:
	/**
	 * @param InActor The actor to monitor. Can be null.
	 */
	explicit FGCFExistPredicate(TWeakObjectPtr<AActor> InActor)
		: Actor(InActor)
	{}

	/**
	 * Checks if the actor is currently valid (not pending kill and not null).
	 */
	virtual bool Evaluate() const override
	{
		return Actor.IsValid();
	}

private:
	/** Weak reference to avoid strong reference cycles and ensure safe access. */
	TWeakObjectPtr<AActor> Actor;
};
