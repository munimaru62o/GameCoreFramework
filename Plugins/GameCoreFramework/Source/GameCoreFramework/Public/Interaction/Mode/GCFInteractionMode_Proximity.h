// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Interaction/Mode/GCFInteractionMode.h"
#include "GCFInteractionMode_Proximity.generated.h"

/**
 * @brief Interaction mode for Third-Person or First-Person views.
 * * Performs a Sphere Sweep from the Camera's viewpoint.
 * Also validates that the target is roughly in front of the Pawn to prevent weird backward interactions.
 */
UCLASS()
class GAMECOREFRAMEWORK_API UGCFInteractionMode_Proximity : public UGCFInteractionMode
{
	GENERATED_BODY()
public:
	virtual AActor* FindTarget(const FGCFInteractionSearchParams& Params) const override;

protected:
	/** Radius of the sphere sweep used for detection. Easier to hit targets than a line trace. */
	UPROPERTY(EditAnywhere, Category = "Settings")
	float SweepRadius = 30.0f;

	/** * Dot product threshold to limit the interaction angle relative to the Pawn's forward vector.
	 * 1.0 = Exactly in front, 0.0 = 90 degrees (Side), -1.0 = Behind.
	 */
	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float AngleThreshold = 0.5f;
};