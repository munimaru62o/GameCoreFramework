// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Camera/Mode/GCFCameraMode_Orbit.h"
#include "Camera/GCFCameraComponent.h"


UGCFCameraMode_Orbit::UGCFCameraMode_Orbit()
{
	FieldOfView = 80.0f;
}

void UGCFCameraMode_Orbit::UpdateView(float DeltaTime)
{
	// 1. Get the base location (Character's head/pivot)
	const FVector PivotLocation = GetPivotLocation();

	// 2. Calculate offset based on the Fixed Pitch
	// The curves (defined in base class) determine the X/Y/Z offset relative to the camera angle.
	const float ZoomRatio = GetCameraZoomRatio();
	const FVector LocalOffset = CalculateOffsetFromCurves(FixedRotation.Pitch) * ZoomRatio;

	// 3. Rotate the offset to match the fixed camera angle
	// This places the camera behind/above the pivot based on the rotation.
	const FVector FinalLocation = PivotLocation + FixedRotation.RotateVector(LocalOffset);

	// 4. Apply final view properties
	View.Location = FinalLocation;
	View.Rotation = FixedRotation;

	// Lock the controller rotation to the camera rotation (Standard for fixed-angle games).
	// Note: If you want "Twin Stick" aiming independent of camera, you might want to decouple this.
	View.ControlRotation = View.Rotation;

	View.FieldOfView = FieldOfView;
}
