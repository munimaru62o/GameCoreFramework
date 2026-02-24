// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Movement/Mover/GCFCharacterMoverComponent.h"
#include "Movement/GCFMovementConfig.h"
#include "Movement/Mover/Input/GCFHumanoidInputs.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"


void UGCFCharacterMoverComponent::ApplyMovementConfig_Implementation(const UGCFMovementConfig* Config)
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


void UGCFCharacterMoverComponent::OnMoverPreSimulationTick(const FMoverTimeStep& TimeStep, const FMoverInputCmdContext& InputCmd)
{
	if (const FGCFHumanoidInputs* HumanoidInputs = InputCmd.InputCollection.FindDataByType<FGCFHumanoidInputs>()) {
		bWantsToCrouch = HumanoidInputs->bWantsToCrouch;
	}

	Super::OnMoverPreSimulationTick(TimeStep, InputCmd);
}