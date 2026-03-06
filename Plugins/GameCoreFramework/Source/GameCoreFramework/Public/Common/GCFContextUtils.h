// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Components/ActorComponent.h"

/**
 * GCF::Context
 * Utilities for resolving and exploring relationships between objects.
 *
 * [Include Cost]
 * This header pulls in Actor.h, Controller.h, and ActorComponent.h.
 * Include only in .cpp files or headers that genuinely need interface resolution.
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
		if (UActorComponent* FoundComponent = Actor->FindComponentByInterface(UInterfaceClass::StaticClass())) {
			return TScriptInterface<InterfaceType>(FoundComponent);
		}
	}

	// 3. If Component, recurse to Owner
	if (const UActorComponent* Component = Cast<UActorComponent>(Context)) {
		return ResolveInterface<InterfaceType, UInterfaceClass>(Component->GetOwner());
	}

	// 4. If Controller, check the Pawn
	if (const AController* Controller = Cast<AController>(Context)) {
		return ResolveInterface<InterfaceType, UInterfaceClass>(Controller->GetPawn());
	}

	return nullptr;
}
}
