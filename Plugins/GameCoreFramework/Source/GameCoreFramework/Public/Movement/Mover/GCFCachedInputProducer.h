// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MoverSimulationTypes.h"
#include "GCFCachedInputProducer.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

/**
 * @brief Mover input producer that bridges the GCF Input System with the Mover plugin.
 *
 * Instead of reading hardware input directly, this producer queries the owning Pawn
 * (via IGCFMovementInputProvider) for its cached movement intention. This allows the
 * project to maintain a data-driven input flow (InputAction -> InputTag -> Pawn Cache)
 * while fully supporting Mover's tick-based prediction and rollback systems.
 */
UCLASS(Blueprintable, ClassGroup = (GCF), meta = (BlueprintSpawnableComponent))
class UGCFCachedInputProducer : public UActorComponent, public IMoverInputProducerInterface
{
    GENERATED_BODY()

public:
	/**
	 * Called every simulation frame by the Mover component to gather input.
	 * Retrieves the cached input vector from the Pawn and injects it into Mover's data model.
	 */
    virtual void ProduceInput_Implementation(int32 SimTime, FMoverInputCmdContext& InputCmdResult) override;
};

#undef UE_API