// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GCFMovementFunctionLibrary.generated.h"

class IGCFMovementConfigReceiver;
class IGCFLocomotionHandler;

#define UE_API GAMECOREFRAMEWORK_API

/**
 * @brief Static function library dedicated to Movement utilities.
 */
UCLASS(Abstract, MinimalAPI)
class UGCFMovementFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Resolves the MovementConfigReceiver Interface.
	 */
	UFUNCTION(BlueprintCallable, Category = "GCF|Movement", meta = (DefaultToSelf = "Context"))
	static TScriptInterface<IGCFMovementConfigReceiver> ResolveMovementConfigReceiver(const UObject* Context);

	/**
	 * Resolves the LocomotionHandler Interface.
	 */
	UFUNCTION(BlueprintCallable, Category = "GCF|Movement", meta = (DefaultToSelf = "Context"))
	static TScriptInterface<IGCFLocomotionHandler> ResolveLocomotionHandler(const UObject* Context);
};

#undef UE_API