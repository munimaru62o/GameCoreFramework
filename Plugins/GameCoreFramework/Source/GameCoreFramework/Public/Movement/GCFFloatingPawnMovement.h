// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Movement/GCFMovementConfigReceiver.h"
#include "GCFFloatingPawnMovement.generated.h"

/**
 * @brief GCF-compliant wrapper for UFloatingPawnMovement.
 * * Extends the standard engine floating movement to implement IGCFMovementConfigReceiver.
 * This allows the component to receive data-driven configuration from UGCFMovementConfig.
 */
UCLASS(ClassGroup = (GCF), Within = Pawn, Blueprintable, Meta = (BlueprintSpawnableComponent))
class GAMECOREFRAMEWORK_API UGCFFloatingPawnMovement : public UFloatingPawnMovement, public IGCFMovementConfigReceiver
{
	GENERATED_BODY()

public:
	//~IGCFMovementConfigReceiver interface
	virtual void ApplyMovementConfig_Implementation(const UGCFMovementConfig* Config) override;
	//~End of IGCFMovementConfigReceiver interface
};