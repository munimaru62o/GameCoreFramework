// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GCFMovementConfig.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

/**
 * @brief Base Data Asset for defining Pawn movement parameters.
 * Used to drive movement components via the IGCFMovementConfigReceiver.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "GCF Movement Config", ShortTooltip = "Base Movement Settings"))
class UE_API UGCFMovementConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Maximum speed (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MaxSpeed = 1200.0f;

	/** Acceleration (cm/s^2). Determines how fast the pawn reaches max speed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	float Acceleration = 4000.0f;

	/** Acceleration (cm/s^2). Determines how fast the pawn reaches max speed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	float Deceleration = 4000.0f;
};


#undef UE_API