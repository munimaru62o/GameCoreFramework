// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Movement/Mover/Producer/GCFCachedInputProducer.h"
#include "GCFHumanoidInputProducer.generated.h"

/**
 * @brief Input producer specifically for Pawns requiring Humanoid inputs (like Crouch).
 * Inherits basic movement logic and Jump actions from GCFCachedInputProducer,
 * and adds additional interface polling for stance changes (Crouch) to inject into FGCFHumanoidInputs.
 */
UCLASS(Blueprintable, ClassGroup = (GCF), meta = (BlueprintSpawnableComponent))
class GAMECOREFRAMEWORK_API UGCFHumanoidInputProducer : public UGCFCachedInputProducer
{
	GENERATED_BODY()

public:
	virtual void ProduceInput_Implementation(int32 SimTime, FMoverInputCmdContext& InputCmdResult) override;
};