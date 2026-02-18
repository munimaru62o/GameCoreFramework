// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Interaction/Mode/GCFInteractionMode_TopDown.h"
#include "GameFramework/PlayerController.h"


AActor* UGCFInteractionMode_TopDown::FindTarget(const FGCFInteractionSearchParams& Params) const
{
	if (!Params.PC) return nullptr;

	FVector WorldLoc, WorldDir;
	
	// Convert Mouse Position to World Ray
	if (Params.PC->DeprojectMousePositionToWorld(WorldLoc, WorldDir)) {
		FHitResult Hit;
		FVector TraceEnd = WorldLoc + (WorldDir * Params.Distance);

		// Execute Trace
		if (PerformTrace(Params, WorldLoc, TraceEnd, 0.0f, Params.Channel, Params.Pawn, Params.bShowDebug, Hit)) {
			return Hit.GetActor();
		}
	}
	return nullptr;
}