// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"

// If build time is a concern, isolate it
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Components/ActorComponent.h"

/**
 * Utility namespace for handling type-safe Enum Class bitmasks.
 * Wraps bitwise operations to provide readable state evaluation logic.
 */
namespace GCF::Bitmask
{
/**
 * Checks if ALL flags specified in the mask are present in the current state.
 *
 * @param State         The current state bitmask.
 * @param RequiredMask  The combination of flags required.
 * @return True only if every bit in RequiredMask is set in State.
 */
template <typename T>
[[nodiscard]] FORCEINLINE bool AreFlagsSet(T State, T RequiredMask)
{
	return (State & RequiredMask) == RequiredMask;
}

/**
 * Checks if the "Satisfaction Status" of a mask has changed between two states.
 * Useful for detecting "Rising Edge" (Just became Ready) or "Falling Edge" (Just lost Ready) events.
 *
 * Note: This does NOT check if raw bits changed. It checks if the condition (AreFlagsSet) result changed.
 *
 * @param OldState      The previous frame or cached state.
 * @param NewState      The current updated state.
 * @param RequiredMask  The condition to evaluate.
 * @return True if the readiness status has flipped (Ready <-> Not Ready).
 */
template <typename T>
[[nodiscard]] FORCEINLINE bool HasFlagsChanged(T OldState, T NewState, T RequiredMask)
{
	const bool bOldMet = AreFlagsSet(OldState, RequiredMask);
	const bool bNewMet = AreFlagsSet(NewState, RequiredMask);

	// Returns true if the state toggled (XOR)
	return bOldMet != bNewMet;
}

/**
 * Helper to check if ANY flag in the mask is set.
 * Useful for broad checks like "Is any error flag set?".
 */
template <typename T>
[[nodiscard]] FORCEINLINE bool IsAnyFlagSet(T State, T Mask)
{
	return (State & Mask) != static_cast<T>(0);
}
}

/**
 * GCF::Context
 * Utilities for resolving and exploring relationships between objects
 */
namespace GCF::Context
{
/**
 * Generic helper to find an interface on an Object, Actor, or its Owner chain.
 * Searches: Object -> Actor's Components -> Component's Owner -> Controller's Pawn
 */
template<typename InterfaceType, typename UInterfaceClass>
static TScriptInterface<InterfaceType> ResolveInterface(const UObject* Context)
{
	if (!Context) return nullptr;

	// 1. Does the object itself implement it?
	if (Context->Implements<UInterfaceClass>()) {
		return TScriptInterface<InterfaceType>(const_cast<UObject*>(Context));
	}

	// 2. If Actor, search components
	if (const AActor* Actor = Cast<AActor>(Context)) {
		if (InterfaceType* FoundInterface = Actor->FindComponentByInterface<InterfaceType>()) {
			if (UObject* FoundObject = Cast<UObject>(FoundInterface)) {
				return TScriptInterface<InterfaceType>(FoundObject);
			}
		}
	}

	// 3. If Component, recurse to Owner
	if (const UActorComponent* Component = Cast<UActorComponent>(Context)) {
		return ResolveInterface<InterfaceType, UInterfaceClass>(Component->GetOwner());
	}

	// 4. If Controller, check the Pawn
	// (Requires definition of AController, hence the include at top)
	if (const AController* Controller = Cast<AController>(Context)) {
		return ResolveInterface<InterfaceType, UInterfaceClass>(Controller->GetPawn());
	}

	return nullptr;
}
}