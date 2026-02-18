// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/Binder/GCFControllerPossessionBinder.h"

#include "GCFShared.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"


FGCFControllerPossessionBinder::FGCFControllerPossessionBinder(UGameFrameworkComponentManager * InGFCM, AController * InController, FGCFBooleanStateSignature && InDelegate)
	: FGCFContextBinder(InGFCM, APawn::StaticClass())
	, Controller(InController)
	, Delegate(MoveTemp(InDelegate))
{};


bool FGCFControllerPossessionBinder::TryResolveEvent(AActor* Actor, FName EventName)
{
	if (EventName == GCF::Names::Event_Pawn_OnPossessed) {
		if (APawn* NewPawn = Cast<APawn>(Actor)) {
			// Check if this pawn is possessed by OUR controller
			if (NewPawn->GetController() == Controller) {
				Pawn = NewPawn;
				Delegate.ExecuteIfBound(Actor, true);
				return true;
			}
		}
	}

	if (EventName == GCF::Names::Event_Pawn_OnUnPossessed) {
		if (APawn* OldPawn = Cast<APawn>(Actor)) {
			// Check if the unpossessed pawn is the one we were tracking
			if (OldPawn == Pawn) {
				Pawn.Reset();
				Delegate.ExecuteIfBound(Actor, false);
				return true;
			}
		}
	}
	return false;
}