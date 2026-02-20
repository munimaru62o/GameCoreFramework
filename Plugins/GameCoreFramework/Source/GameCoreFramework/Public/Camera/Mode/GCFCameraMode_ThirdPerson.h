// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Camera/Mode/GCFCameraMode.h"
#include "GCFCameraMode_ThirdPerson.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

/**
 * @brief Standard Third-Person Camera Mode.
 *
 * Controls the camera based on the Controller's rotation (ControlRotation).
 * Distance and offsets are determined by the curves defined in the base class (TargetOffset),
 * allowing for dynamic camera positioning based on the view pitch.
 *
 * Note: Currently does not handle collision (camera clipping through walls).
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class UE_API UGCFCameraMode_ThirdPerson : public UGCFCameraMode
{
	GENERATED_BODY()

public:
	UGCFCameraMode_ThirdPerson();

protected:
	virtual void UpdateView(float DeltaTime) override;
};

#undef UE_API