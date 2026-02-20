// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Interaction/Mode/GCFInteractionMode_Proximity.h"
#include "GameFramework/PlayerController.h"

AActor* UGCFInteractionMode_Proximity::FindTarget(const FGCFInteractionSearchParams& Params) const
{
	if (!Params.PC || !Params.Pawn) return nullptr;

	// Calculate trace from Camera Viewpoint
	FVector Start; FRotator Rot;
	Params.PC->GetPlayerViewPoint(Start, Rot);
	FVector End = Start + (Rot.Vector() * Params.Distance);

	FHitResult Hit;
	// Execute Sweep
	if (PerformTrace(Params, Start, End, SweepRadius, Params.Channel, Params.Pawn, Params.bShowDebug, Hit)) {
		if (AActor* HitActor = Hit.GetActor()) {
			// Validate Angle: Check if the target is within the allowed angle relative to the Pawn's forward direction.
			FVector ToTarget = (HitActor->GetActorLocation() - Params.Pawn->GetActorLocation()).GetSafeNormal();
			if (FVector::DotProduct(Params.Pawn->GetActorForwardVector(), ToTarget) >= AngleThreshold) {
				return HitActor;
			}
		}
	}
	return nullptr;
}