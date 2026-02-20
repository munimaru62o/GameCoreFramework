// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GCFActorFunctionLibrary.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class APlayerState;
class AController;
class APawn;
class IGCFPawnDataProvider;


/**
 * @brief Static function library dedicated to Actor utilities.
 */
UCLASS(Abstract, MinimalAPI)
class UGCFActorFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Resolves the APlayerState associated with the given context.
	 * * Search Order:
	 * 1. Context is PlayerState -> Return it.
	 * 2. Context is Pawn -> Return Pawn->GetPlayerState().
	 * 3. Context is Controller -> Return Controller->PlayerState.
	 * 4. Context is Component -> Recursive search on Owner.
	 */
	UFUNCTION(BlueprintCallable, Category = "GCF|Actor", meta = (DefaultToSelf = "Context"))
	static APlayerState* ResolvePlayerState(UObject* Context);

	/**
	 * Resolves the AController associated with the given context.
	 */
	UFUNCTION(BlueprintCallable, Category = "GCF|Actor", meta = (DefaultToSelf = "Context"))
	static const AController* ResolveController(const UObject* Context);

	/**
	 * Resolves the APawn associated with the given context.
	 */
	UFUNCTION(BlueprintCallable, Category = "GCF|Actor", meta = (DefaultToSelf = "Context"))
	static const APawn* ResolvePawn(const UObject* Context);

	/**
	 * Resolves the Pawn Data Provider interface.
	 * Useful for retrieving PawnData without knowing the exact Pawn class.
	 */
	UFUNCTION(BlueprintCallable, Category = "GCF|Actor", meta = (DefaultToSelf = "Context"))
	static TScriptInterface<IGCFPawnDataProvider> ResolvePawnDataProvider(const UObject* Context);
};

#undef UE_API