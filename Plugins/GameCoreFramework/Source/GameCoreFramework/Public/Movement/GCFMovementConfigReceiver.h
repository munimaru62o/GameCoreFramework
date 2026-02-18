// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GCFMovementConfigReceiver.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UGCFMovementConfig;

UINTERFACE(MinimalAPI, Blueprintable)
class UGCFMovementConfigReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief Interface for Movement Components to accept data-driven configurations.
 * * Implemented by components (like GCFCharacterMovementComponent) that need to
 * apply speed, acceleration, or other physics settings from a shared config asset.
 */
class UE_API IGCFMovementConfigReceiver
{
	GENERATED_BODY()

public:
	/**
	 * Applies the parameters defined in the config object to the movement component.
	 * @param Config The movement configuration asset.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Movement")
	void ApplyMovementConfig(const UGCFMovementConfig* Config);
};

#undef UE_API
