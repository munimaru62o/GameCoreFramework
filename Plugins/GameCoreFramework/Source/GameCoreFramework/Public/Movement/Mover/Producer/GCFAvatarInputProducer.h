// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Movement/Mover/Producer/GCFCachedInputProducer.h"
#include "GCFAvatarInputProducer.generated.h"

/**
 * @brief Input producer specifically for Avatar Pawns.
 * Inherits basic movement vector logic from GCFCachedInputProducer,
 * and adds Avatar-specific actions like Jump and Crouch.
 */
UCLASS(Blueprintable, ClassGroup = (GCF), meta = (BlueprintSpawnableComponent))
class GAMECOREFRAMEWORK_API UGCFAvatarInputProducer : public UGCFCachedInputProducer
{
	GENERATED_BODY()

public:
	virtual void ProduceInput_Implementation(int32 SimTime, FMoverInputCmdContext& InputCmdResult) override;
};