// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Movement/GCFFloatingPawnMovement.h"

#include "GCFShared.h"
#include "Movement/GCFMovementConfig.h"


void UGCFFloatingPawnMovement::ApplyMovementConfig_Implementation(const UGCFMovementConfig* Config)
{
	if (!Config) {
		return;
	}
	// Apply base settings
	MaxSpeed = Config->MaxSpeed;
	Acceleration = Config->Acceleration;
	Deceleration = Config->Deceleration;
}

