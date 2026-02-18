// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/Binder/GCFPawnControllerAssignedBinder.h"

#include "GCFShared.h"
#include "GameFramework/Pawn.h"


FGCFPawnControllerAssignedBinder::FGCFPawnControllerAssignedBinder(UGameFrameworkComponentManager* InGFCM, APawn* InPawn, FGCFBooleanStateSignature&& InDelegate)
	: FGCFBooleanStateBinder(
		InGFCM,
		APawn::StaticClass(),
		InPawn,
		GCF::Names::Event_Pawn_ControllerAssigned,
		NAME_None, // Only listening for the "On" event (Assigned)
		MoveTemp(InDelegate))
{}


bool FGCFPawnControllerAssignedBinder::TryResolveImmediate()
{
	// Check if the specific pawn already has a controller assigned
	if (APawn* Pawn = Cast<APawn>(SpecificActor.Get())) {
		if (Pawn->GetController()) {
			Delegate.ExecuteIfBound(Pawn, true);
			return true;
		}
	}
	return false;
}