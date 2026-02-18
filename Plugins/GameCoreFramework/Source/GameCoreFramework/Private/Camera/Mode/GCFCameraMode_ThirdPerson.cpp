// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Camera/Mode/GCFCameraMode_ThirdPerson.h"
#include "Camera/GCFCameraComponent.h"


UGCFCameraMode_ThirdPerson::UGCFCameraMode_ThirdPerson()
{
	FieldOfView = 80.0f;
}


void UGCFCameraMode_ThirdPerson::UpdateView(float DeltaTime)
{
	// 1. Get Base Anchor (Target's location)
	const FVector PivotLocation = GetPivotLocation();

	// 2. Get Input Rotation (Control Rotation)
	// This allows the player to rotate the camera around the character.
	FRotator PivotRotation = GetPivotRotation();
	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	// 3. Calculate Offset
	// Curves determine the offset based on the current Pitch (e.g. move closer when looking up).
	// ZoomRatio scales the entire distance.
	const float ZoomRatio = GetCameraZoomRatio();
	const FVector LocalOffset = CalculateOffsetFromCurves(PivotRotation.Pitch) * ZoomRatio;

	// 4. Apply Rotation to Offset (Local -> World)
	// Transform the local offset (Camera Space) into World Space using the Control Rotation.
	const FVector FinalLocation = PivotLocation + PivotRotation.RotateVector(LocalOffset);

	// 5. Finalize View
	View.Location = FinalLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
}