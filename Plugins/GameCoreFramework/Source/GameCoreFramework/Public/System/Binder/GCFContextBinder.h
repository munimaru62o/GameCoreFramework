// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"

class UGameFrameworkComponentManager;
struct FComponentRequestHandle;

/**
 *   Base class for managing the lifecycle of GameFrameworkComponentManager (GFCM) extension events.
 * * Features:
 * - RAII Pattern: Automatically unregisters the extension handler when this object is destroyed.
 * - Fast Path: Checks if the condition is already met (TryResolveImmediate) before registering a listener.
 * - Safety: Prevents pure virtual function calls during destruction by providing default implementations.
 */
class FGCFContextBinder
{
public:
	/**
	 * Destructor.
	 * Automatically calls Deactivate() to unregister from GFCM.
	 */
	virtual ~FGCFContextBinder() { Deactivate(); }

	/**
	 * Starts monitoring the target actor/class.
	 * 1. Tries to resolve immediately (Fast Path).
	 * 2. If failed, registers an extension handler to GFCM (Slow Path).
	 */
	void Activate();

	/**
	 * Manually unregisters the extension handler.
	 * Called automatically by the destructor.
	 */
	virtual void Deactivate();

protected:
	/**
	 * Constructor.
	 */
	FGCFContextBinder(UGameFrameworkComponentManager* InGFCM, const TSoftClassPtr<AActor>& InReceiverClass);

	/**
	 * Checks if the condition is already met without waiting for an event.
	 * @return true if resolved and no further monitoring is needed.
	 */
	virtual bool TryResolveImmediate() { return false; }

	/**
	 * Resolves the condition based on the received actor and event.
	 * * @note provided with an empty implementation to prevent "Pure Virtual Function Call" crashes
	 * if triggered during the destruction phase.
	 */
	virtual bool TryResolveEvent(AActor* Actor, FName EventName) { return false; };

	/**
	 * Internal callback invoked by the GameFrameworkComponentManager (GFCM).
	 * This serves as the target for the extension handler delegate registered via AddExtensionHandler.
	 *
	 * @param Actor     The actor instance associated with the extension event.
	 * @param EventName The specific event tag (e.g., "GameFeature.Player.Ready").
	 */
	virtual void HandleExtension(AActor* Actor, FName EventName);

protected:
	TWeakObjectPtr<UGameFrameworkComponentManager> GFCM;
	TSoftClassPtr<AActor> ReceiverClass;
	TSharedPtr<FComponentRequestHandle> ExtensionHandle;
};
