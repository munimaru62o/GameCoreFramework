// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/Binder/GCFPawnReadyStateBinder.h"

#include "GCFShared.h"
#include "GameFramework/Pawn.h"
#include "System/Lifecycle/GCFStateFunctionLibrary.h"


FGCFPawnReadyStateBinder::FGCFPawnReadyStateBinder(UGameFrameworkComponentManager* InGFCM, APawn* InPawn, FGCFOnPawnReadyStateChangedNative::FDelegate&& InDelegate)
	: FGCFContextBinder(InGFCM, APawn::StaticClass())
	, Pawn(InPawn)
	, Delegate(MoveTemp(InDelegate))
{};


void FGCFPawnReadyStateBinder::Deactivate()
{
	TrackerHandle.Reset();
	FGCFContextBinder::Deactivate();
}


bool FGCFPawnReadyStateBinder::TryResolveImmediate()
{
	// If the pawn exists, try to establish the scoped binding via the helper library.
	// This returns a handle that manages the delegate's lifecycle.
	if (Pawn.Get()) {
		TrackerHandle = UGCFStateFunctionLibrary::BindPawnReadyStateScoped(Pawn.Get(), Delegate);
		return true;
	}
	return false;
}


bool FGCFPawnReadyStateBinder::TryResolveEvent(AActor* Actor, FName EventName)
{
	// Ensure the event belongs to the specific Pawn we are tracking
	if (Actor && Actor == Pawn.Get()) {
		return TryResolveImmediate();
	}
	return false;
}