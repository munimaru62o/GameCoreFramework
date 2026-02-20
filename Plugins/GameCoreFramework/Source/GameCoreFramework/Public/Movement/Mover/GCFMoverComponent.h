// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "MoverComponent.h"
#include "Movement/GCFMovementConfigReceiver.h"
#include "GCFMoverComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UGCFMovementConfig;

/**
 * @brief Wrapper component that integrates the Mover plugin with the GCF Data-Driven architecture.
 *
 * This component acts as a bridge between the data layer (UGCFMovementConfig) and the
 * physics-based Mover system, automatically injecting parameter changes (MaxSpeed, etc.)
 * into Mover's shared settings.
 */
UCLASS(Blueprintable, ClassGroup = (GCF), meta = (BlueprintSpawnableComponent))
class UGCFMoverComponent : public UMoverComponent, public IGCFMovementConfigReceiver
{
    GENERATED_BODY()

public:
	/**
	 * Called to apply data-driven movement parameters to this component.
	 * Overrides the interface method to update Mover's internal shared settings.
	 */
    virtual void ApplyMovementConfig_Implementation(const UGCFMovementConfig* Config) override;
};

#undef UE_API