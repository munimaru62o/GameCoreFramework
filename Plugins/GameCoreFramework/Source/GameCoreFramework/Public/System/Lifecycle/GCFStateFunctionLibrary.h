// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "System/Lifecycle/GCFStateTypes.h" 
#include "Common/GCFTypes.h" 
#include "GCFStateFunctionLibrary.generated.h"

class APawn;
class APlayerState;
class AController;
class FGCFDelegateHandle;

#define UE_API GAMECOREFRAMEWORK_API

/**
 * @brief Static function library dedicated to State monitoring and Lifecycle events.
 * * [Purpose]
 * Provides convenient, scoped (RAII) bindings for:
 * - Pawn Readiness (Body initialization)
 * - Player Readiness (Soul initialization)
 * - Controller Possession changes (Soul entering/leaving Body)
 * * [Usage]
 * The returned TUniquePtr<FGCFDelegateHandle> must be stored by the caller.
 * When the handle goes out of scope, the delegate is automatically unbound.
 */
UCLASS(Abstract, MinimalAPI)
class UGCFStateFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Binds a delegate to the Pawn's "Ready State" changes (e.g., Initialized, Possessed, GameplayReady).
	 * * @param Pawn                 The Pawn to observe.
	 * @param Delegate             The function to call when state changes.
	 * @param bExecuteImmediately  If true, checks the current state and fires the delegate immediately.
	 * @return A scoped handle. The binding is removed when this handle is destroyed.
	 */
	UE_API static TUniquePtr<FGCFDelegateHandle> BindPawnReadyStateScoped(APawn* Pawn, const FGCFOnPawnReadyStateChangedNative::FDelegate& Delegate, bool bExecuteImmediately = true);

	/**
	 * Binds a delegate to the PlayerState's "Ready State" changes.
	 * * @param PlayerState          The PlayerState to observe.
	 * @param Delegate             The function to call when state changes.
	 * @param bExecuteImmediately  If true, checks the current state and fires the delegate immediately.
	 * @return A scoped handle.
	 */
	UE_API static TUniquePtr<FGCFDelegateHandle> BindPlayerReadyStateScoped(APlayerState* PlayerState, const FGCFOnPlayerReadyStateChangedNative::FDelegate& Delegate, bool bExecuteImmediately = true);

	/**
	 * Binds a delegate to monitor Possession changes on a Controller.
	 * Useful for systems that need to know when the Controller possesses a new Pawn (or unpossesses).
	 * * @param Controller           The Controller to observe.
	 * @param Delegate             The function to call when possession changes.
	 * @param bExecuteImmediately  If true, fires the delegate immediately with the current Pawn (if any).
	 * @return A scoped handle.
	 */
	UE_API static TUniquePtr<FGCFDelegateHandle> BindPossessionScoped(AController* Controller, const FOnPossessedPawnChangedNative::FDelegate& Delegate, bool bExecuteImmediately = true);
};

#undef UE_API