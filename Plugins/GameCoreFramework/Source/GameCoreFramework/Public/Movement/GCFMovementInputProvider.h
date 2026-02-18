// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "GCFShared.h"
#include "UObject/Interface.h"
#include "GCFMovementInputProvider.generated.h"

class UGCFInputConfig;


UINTERFACE(MinimalAPI)
class UGCFMovementInputProvider : public UInterface
{
	GENERATED_BODY()
};


class GAMECOREFRAMEWORK_API IGCFMovementInputProvider
{
	GENERATED_BODY()

public:
    /** 
     * Returns the world-space direction/magnitude the player intends to move.
     * This is usually cached from the Controller's input.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Movement")
    FVector GetDesiredMovementVector() const;
};