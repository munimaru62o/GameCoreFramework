// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Input/GCFInputTypes.h"
#include "GCFInputFunctionLibrary.generated.h"

class IGCFInputConfigProvider;
class AController;
class FGCFDelegateHandle;
class UObject;

#define UE_API GAMECOREFRAMEWORK_API

/**
 * @brief Static function library dedicated to Input System utilities.
 * * [Purpose]
 * Centralizes logic for:
 * - Resolving Input Config Providers (finding who holds the input mapping).
 * - Binding to Input Context changes (Gatekeeper status).
 * - Binding to InputComponent initialization events.
 */
UCLASS(Abstract, MinimalAPI)
class UGCFInputFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Resolves the InputConfigProvider interface based on the specified SourceType.
	 * * @param Context     The starting object for the search.
	 * @param SourceType  Whether to look for 'Player' (Controller) or 'Pawn' (Character) configs.
	 * @return The interface found, or nullptr.
	 */
	UFUNCTION(BlueprintCallable, Category = "GCF|Input", meta = (DefaultToSelf = "Context"))
	static TScriptInterface<IGCFInputConfigProvider> ResolveInputConfigProvider(const UObject* Context, EGCFInputSourceType SourceType);

	/**
	 * Binds a delegate to monitor the "Input Context State" (Gatekeeper status).
	 * Useful for UI or Systems that need to know if input is currently blocked or allowed.
	 * * @param Controller           The Controller owning the InputContextComponent.
	 * @param Delegate             Callback function.
	 * @param bExecuteImmediately  If true, fires immediately with the current state.
	 * @return Scoped handle for auto-unbinding.
	 */
	UE_API static TUniquePtr<FGCFDelegateHandle> BindInputContextScoped(AController* Controller, const FOnInputContextEvaluatedNative::FDelegate& Delegate, bool bExecuteImmediately = true);

	/**
	 * Binds a delegate to be notified when the EnhancedInputComponent is ready/changed.
	 * * @param Controller           The GCFPlayerController.
	 * @param Delegate             Callback function receiving the UInputComponent*.
	 * @param bExecuteImmediately  If true, fires immediately if InputComponent is already valid.
	 * @return Scoped handle for auto-unbinding.
	 */
	UE_API static TUniquePtr<FGCFDelegateHandle> BindInputComponentReadyScoped(AController* Controller, const FOnInputComponentReady::FDelegate& Delegate, bool bExecuteImmediately = true);
};

#undef UE_API