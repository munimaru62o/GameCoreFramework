// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Movement/Mover/GCFMoverComponent.h"
#include "Movement/GCFMovementConfig.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"


void UGCFMoverComponent::ApplyMovementConfig_Implementation(const UGCFMovementConfig* Config)
{
	if (!Config) {
		return;
	}

	// TODO: If this logic needs to be shared across multiple Mover derived classes in the future, 
	// consider extracting it into a separate handler component or function library.
	if (UCommonLegacyMovementSettings* LegacyMovementSettings = FindSharedSettings_Mutable<UCommonLegacyMovementSettings>()) {

		// Inject the data-driven config values into Mover's shared settings
		LegacyMovementSettings->MaxSpeed = Config->MaxSpeed;
		LegacyMovementSettings->Acceleration = Config->Acceleration;
		LegacyMovementSettings->Deceleration = Config->Deceleration;
	}
}