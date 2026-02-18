// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "GCFDummyMovementMode.generated.h"

/**
 * A completely empty dummy movement mode existing solely to maintain Network Prediction Plugin (NPP) clock synchronization.
 * * By assigning this mode to a dummy MoverComponent on the PlayerController, we ensure the NPP clock
 * continues to tick for the Autonomous Proxy. It intentionally contains absolutely zero physics calculations
 * and requires no dependent configuration assets (e.g., UCommonLegacyMovementSettings).
 * This makes it an extremely lightweight and safe structural workaround for hybrid movement architectures.
 */
UCLASS()
class UGCFDummyMovementMode : public UBaseMovementMode
{
	GENERATED_BODY()

public:
	// Generates movement intent. (Intentionally does nothing)
	virtual void GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override
	{
		// Return an empty ProposedMove to ensure absolutely zero movement intent is generated.
	}

	// Executes the actual physics simulation. (Intentionally does nothing)
	virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override
	{
		// Bypass all physics calculations and leave the physical state completely unmodified.
	}
};