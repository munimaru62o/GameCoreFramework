// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "DefaultMovementSet/Modes/FallingMode.h"
#include "GCFFallingMode.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

struct FMoverTickStartData;
struct FMoverTimeStep;
struct FProposedMove;

/**
 * @brief Custom flying mode extending the base Mover UFallingMode.
 *
 * This mode includes critical network prediction safety measures.
 * It prevents simulated proxies from continuously extrapolating stale inputs
 * (ghost movement) during packet loss.
 */
UCLASS(Blueprintable, BlueprintType)
class GAMECOREFRAMEWORK_API UGCFFallingMode : public UFallingMode
{
    GENERATED_BODY()

public:
	/**
	  * Core function called every frame by the Mover component to generate physical movement.
	  * Overridden here to sanitize input buffers for simulated proxies before physics execution.
	  */
    virtual void GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;
};

#undef UE_API