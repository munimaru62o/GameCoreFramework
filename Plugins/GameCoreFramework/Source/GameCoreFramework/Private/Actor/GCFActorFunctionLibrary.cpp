// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Actor/GCFActorFunctionLibrary.h"

#include "GCFShared.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"

#include "Actor/Data/GCFPawnDataProvider.h"


APlayerState* UGCFActorFunctionLibrary::ResolvePlayerState(UObject* Context)
{
	if (!Context) return nullptr;

	// 1. Direct Cast
	if (APlayerState* PS = Cast<APlayerState>(Context)) return PS;

	// 2. Pawn's PlayerState
	if (const APawn* Pawn = Cast<APawn>(Context)) {
		return Pawn->GetPlayerState();
	}

	// 3. Controller's PlayerState
	if (const AController* Controller = Cast<AController>(Context)) {
		return Controller->PlayerState;
	}

	// 4. Component's Owner
	if (const UActorComponent* Component = Cast<UActorComponent>(Context)) {
		return ResolvePlayerState(Component->GetOwner());
	}

	// 5. Actor's Owner (Recursive)
	if (const AActor* Actor = Cast<AActor>(Context)) {
		AActor* Owner = Actor->GetOwner();
		// Guard against infinite loop if Owner is self
		if (Owner && Owner != Actor) {
			return ResolvePlayerState(Owner);
		}
	}
	return nullptr;
}


const AController* UGCFActorFunctionLibrary::ResolveController(const UObject* Context)
{
	if (!Context) return nullptr;

	// 1. Direct Cast
	if (const AController* Controller = Cast<AController>(Context)) return Controller;

	// 2. Pawn's Controller
	if (const APawn* Pawn = Cast<APawn>(Context)) {
		if (AController* Controller = Pawn->GetController()) return Controller;
	}

	// 3. Component's Owner
	if (const UActorComponent* Component = Cast<UActorComponent>(Context)) {
		return ResolveController(Component->GetOwner());
	}

	// 4. Actor's Owner (Recursive)
	if (const AActor* Actor = Cast<AActor>(Context)) {
		AActor* Owner = Actor->GetOwner();
		// Guard against infinite loop if Owner is self (shouldn't happen but safe)
		if (Owner && Owner != Actor) {
			return ResolveController(Owner);
		}
	}
	return nullptr;
}


const APawn* UGCFActorFunctionLibrary::ResolvePawn(const UObject* Context)
{
	if (!Context) return nullptr;

	// 1. Direct Cast
	if (const APawn* Pawn = Cast<APawn>(Context)) return Pawn;

	// 2. Controller's Pawn
	if (const AController* Controller = Cast<AController>(Context)) {
		return Controller->GetPawn();
	}

	// 3. Component's Owner
	if (const UActorComponent* Component = Cast<UActorComponent>(Context)) {
		return ResolvePawn(Component->GetOwner());
	}

	return nullptr;
}


TScriptInterface<IGCFPawnDataProvider> UGCFActorFunctionLibrary::ResolvePawnDataProvider(const UObject* Context)
{
	return GCF::Context::ResolveInterface<IGCFPawnDataProvider, UGCFPawnDataProvider>(Context);
}
