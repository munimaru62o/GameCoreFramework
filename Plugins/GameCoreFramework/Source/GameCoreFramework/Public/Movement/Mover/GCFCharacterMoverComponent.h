// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "Movement/GCFMovementConfigReceiver.h"
#include "GCFCharacterMoverComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UGCFMovementConfig;

/**
 * @brief Wrapper component that integrates the Mover plugin with the GCF Data-Driven architecture.
 *
 * This component acts as a bridge between the configuration data layer (UGCFMovementConfig) and the
 * physics-based Mover system. It listens for configuration updates and automatically injects
 * parameter changes (e.g., MaxSpeed, Acceleration) into the Mover's Shared Settings.
 */
UCLASS(Blueprintable, ClassGroup = (GCF), meta = (BlueprintSpawnableComponent))
class UGCFCharacterMoverComponent : public UCharacterMoverComponent, public IGCFMovementConfigReceiver
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