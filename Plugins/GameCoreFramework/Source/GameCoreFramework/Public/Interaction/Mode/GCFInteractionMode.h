// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GCFInteractionMode.generated.h"

class UGCFInteractionComponent;

/**
 * @brief Parameters required for detecting interactable targets.
 * Passed to the FindTarget method to provide context (Controller, Pawn, settings).
 */
struct FGCFInteractionSearchParams
{
	const UGCFInteractionComponent* SourceComponent;
	const APlayerController* PC;
	const APawn* Pawn;
	float Distance;
	ECollisionChannel Channel;
	bool bShowDebug;
};

/**
 * @brief Abstract base class for interaction detection strategies.
 * * Defines the interface for "how to find a target".
 * Subclasses implement specific logic like "Raycast from Camera" or "Mouse Cursor Trace".
 * Marked as EditInlineNew to allow property editing directly within DataAssets.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class GAMECOREFRAMEWORK_API UGCFInteractionMode : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Pure virtual function to execute the detection logic.
	 * @param Params Context parameters for the search.
	 * @return The detected actor, or nullptr if nothing valid was found.
	 */
	virtual AActor* FindTarget(const FGCFInteractionSearchParams& Params) const PURE_VIRTUAL(UGCFInteractionMode::FindTarget, return nullptr;);

protected:
	/**
	 * Helper function to execute the actual LineTrace or Sweep.
	 * Handles debug drawing if requested.
	 */
	bool PerformTrace(const FGCFInteractionSearchParams& SearchParams, const FVector& Start, const FVector& End, float Radius, ECollisionChannel Channel, const AActor* IgnoreActor, bool bShowDebug, FHitResult& OutHit) const;
};