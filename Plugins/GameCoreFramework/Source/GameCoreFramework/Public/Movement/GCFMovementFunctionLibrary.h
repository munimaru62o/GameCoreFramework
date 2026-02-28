// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GCFMovementFunctionLibrary.generated.h"

class IGCFMovementConfigReceiver;
class IGCFLocomotionInputHandler;

/**
 * @brief Static function library dedicated to Movement utilities.
 */
UCLASS(Abstract)
class GAMECOREFRAMEWORK_API UGCFMovementFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Resolves the MovementConfigReceiver Interface.
	 */
	UFUNCTION(BlueprintCallable, Category = "GCF|Movement", meta = (DefaultToSelf = "Context"))
	static TScriptInterface<IGCFMovementConfigReceiver> ResolveMovementConfigReceiver(const UObject* Context);

	/**
	 * Resolves the LocomotionInputHandler Interface.
	 */
	UFUNCTION(BlueprintCallable, Category = "GCF|Movement", meta = (DefaultToSelf = "Context"))
	static TScriptInterface<IGCFLocomotionInputHandler> ResolveLocomotionInputHandler(const UObject* Context);
};