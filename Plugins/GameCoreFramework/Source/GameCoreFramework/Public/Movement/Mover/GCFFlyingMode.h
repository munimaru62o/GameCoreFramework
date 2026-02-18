// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "DefaultMovementSet/Modes/FlyingMode.h"
#include "GCFFlyingMode.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

struct FMoverTickStartData;
struct FMoverTimeStep;
struct FProposedMove;

/**
 * @brief Custom flying mode extending the base Mover UFlyingMode.
 *
 * This mode includes critical network prediction safety measures.
 * It prevents simulated proxies from continuously extrapolating stale inputs
 * (ghost movement) during packet loss.
 */
UCLASS(Blueprintable, BlueprintType)
class GAMECOREFRAMEWORK_API UGCFFlyingMode : public UFlyingMode
{
    GENERATED_BODY()

public:
	/**
	  * Core function called every frame by the Mover component to generate physical movement.
	  * Overridden here to sanitize input buffers for simulated proxies before physics execution.
	  */
    virtual void GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, OUT FProposedMove& OutProposedMove) const override;
};

#undef UE_API