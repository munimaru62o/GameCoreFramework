// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Interaction/Mode/GCFInteractionMode.h"
#include "DrawDebugHelpers.h"
#include "Interaction/GCFInteractionComponent.h"


bool UGCFInteractionMode::PerformTrace(const FGCFInteractionSearchParams& SearchParams, const FVector& Start, const FVector& End, float Radius, ECollisionChannel Channel, const AActor* IgnoreActor, bool bShowDebug, FHitResult& OutHit) const
{
	if (!SearchParams.PC) {
		return false;
	}

	const UWorld* World = SearchParams.PC->GetWorld();
	if (!World) {
		return false;
	}

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(IgnoreActor);

	// Choose between Sphere Sweep or Line Trace based on Radius
	const bool bHit = (Radius > 0.0f)
		? World->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, Channel, FCollisionShape::MakeSphere(Radius), Params)
		: World->LineTraceSingleByChannel(OutHit, Start, End, Channel, Params);

#if !UE_BUILD_SHIPPING
	if (bShowDebug && SearchParams.SourceComponent) {
		// Calculate debug line lifetime to match the tick interval of the component.
		// This prevents flickering or cluttering.
		float DebugLifeTime = SearchParams.SourceComponent->PrimaryComponentTick.TickInterval;

		// Fallback if TickInterval is 0 (every frame)
		if (DebugLifeTime <= 0.0f) {
			DebugLifeTime = -1.0f; // Live for one frame
		}

		const FColor TraceColor = bHit ? FColor::Green : FColor::Red;
		if (Radius > 0.0f) {
			// Debug Draw: Sphere Sweep
			// Draw the sphere at the hit location (or end if no hit)
			DrawDebugSphere(World, bHit ? OutHit.Location : End, Radius, 8, TraceColor, false, DebugLifeTime);
			DrawDebugLine(World, Start, bHit ? OutHit.Location : End, TraceColor, false, DebugLifeTime);
		} else {
			// Debug Draw: Line Trace
			DrawDebugLine(World, Start, bHit ? OutHit.ImpactPoint : End, TraceColor, false, DebugLifeTime);
		}

		// Mark the specific impact point
		if (bHit) {
			DrawDebugPoint(World, OutHit.ImpactPoint, 10.0f, TraceColor, false, DebugLifeTime);
		}
	}
#endif
	return bHit;
}