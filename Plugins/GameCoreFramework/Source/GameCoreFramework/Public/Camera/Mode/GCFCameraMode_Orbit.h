// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Camera/Mode/GCFCameraMode.h"
#include "GCFCameraMode_Orbit.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

/**
 * @brief Camera mode that maintains a fixed rotation relative to the world or target.
 *
 * This mode is ideal for:
 * - Top-Down views (Pitch = -90)
 * - Isometric/Quarter views (Pitch = -45, Yaw = 45)
 * - Side Scrollers (Pitch = 0, Yaw = -90)
 *
 * It uses the 'TargetOffset' curves defined in the base class to determine the camera distance
 * based on the current Pitch, allowing for dynamic zooming or height adjustments.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class UE_API UGCFCameraMode_Orbit : public UGCFCameraMode
{
	GENERATED_BODY()

public:
	UGCFCameraMode_Orbit();

protected:
	virtual void UpdateView(float DeltaTime) override;

protected:
	/**
	 * The fixed rotation to apply to the camera in World Space.
	 * - (-90, 0, 0) for Top-Down.
	 * - (-15, 0, 0) for Third-Person (Fixed Angle).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Orbit")
	FRotator FixedRotation = FRotator(-60.0f, 0.0f, 0.0f);
};

#undef UE_API