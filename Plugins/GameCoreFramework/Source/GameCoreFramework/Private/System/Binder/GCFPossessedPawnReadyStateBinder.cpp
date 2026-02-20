// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/Binder/GCFPossessedPawnReadyStateBinder.h"

#include "GCFShared.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "System/Lifecycle/GCFStateFunctionLibrary.h"


FGCFPossessedPawnReadyStateBinder::FGCFPossessedPawnReadyStateBinder(UGameFrameworkComponentManager* InGFCM, AController* InController, FGCFOnPawnReadyStateChangedNative::FDelegate&& InDelegate)
	: FGCFContextBinder(InGFCM, APawn::StaticClass())
	, Controller(InController)
	, Delegate(MoveTemp(InDelegate))
{};


void FGCFPossessedPawnReadyStateBinder::Deactivate()
{
	TrackerHandle.Reset();
	FGCFContextBinder::Deactivate();
}


bool FGCFPossessedPawnReadyStateBinder::TryResolveImmediate()
{
	if (Controller.Get()) {
		if (APawn* CurrentPawn = Controller->GetPawn()) {
			Pawn = CurrentPawn;
			// Bind to the current pawn immediately
			TrackerHandle = UGCFStateFunctionLibrary::BindPawnReadyStateScoped(CurrentPawn, Delegate);
		}
	}

	// [Important] Always return false!
	// Even if we bound to the current pawn, we MUST continue listening to events
	// because the player might hop into a vehicle or respawn later.
	// Returning 'true' would stop the base class from registering the extension handler.
	return false;
}


bool FGCFPossessedPawnReadyStateBinder::TryResolveEvent(AActor* Actor, FName EventName)
{
	// Handle Possession: A pawn was possessed by someone
	if (EventName == GCF::Names::Event_Pawn_OnPossessed) {
		if (APawn* NewPawn = Cast<APawn>(Actor)) {
			// Was it OUR controller?
			if (NewPawn->GetController() == Controller) {

				// Switch binding to the new pawn
				Pawn = NewPawn;
				TrackerHandle = UGCFStateFunctionLibrary::BindPawnReadyStateScoped(NewPawn, Delegate);
				return true;
			}
		}
	}

	// Handle Unpossession: A pawn was unpossessed
	if (EventName == GCF::Names::Event_Pawn_OnUnPossessed) {
		if (APawn* OldPawn = Cast<APawn>(Actor)) {
			// Was it the pawn we were tracking?
			if (OldPawn == Pawn) {
				// Unbind (stop listening to the old pawn's state)
				Pawn.Reset();
				TrackerHandle.Reset();
				return true;
			}
		}
	}
	return false;
}