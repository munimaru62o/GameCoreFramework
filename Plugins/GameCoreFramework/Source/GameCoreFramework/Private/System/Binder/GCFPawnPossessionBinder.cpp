// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/Binder/GCFPawnPossessionBinder.h"

#include "GCFShared.h"
#include "GameFramework/Pawn.h"


FGCFPawnPossessionBinder::FGCFPawnPossessionBinder(UGameFrameworkComponentManager* InGFCM, APawn* InSpecificActor, FGCFBooleanStateSignature&& InDelegate)
	: FGCFBooleanStateBinder(
		InGFCM,
		APawn::StaticClass(),
		InSpecificActor,
		GCF::Names::Event_Pawn_OnPossessed,
		GCF::Names::Event_Pawn_OnUnPossessed,
		MoveTemp(InDelegate))
{};
