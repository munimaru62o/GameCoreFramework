// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "System/Binder/GCFContextBinder.h"
#include "System/Lifecycle/GCFStateTypes.h"

class UGameFrameworkComponentManager;
class FGCFDelegateHandle;

/**
 * A specialized binder that waits for a Controller to acquire a valid PlayerState,
 * and then monitors that PlayerState for its "Ready" status.
 *
 * Workflow:
 * 1. Checks if the Controller already has a PlayerState (TryResolveImmediate).
 * 2. If not, listens for GFCM extension events on APlayerState actors.
 * 3. Once the correct PlayerState is found (ownership check), it initiates a scoped binding
 * via UGCFComponentFunctionLibrary to track the actual readiness logic.
 *
 * Features:
 * - Ownership Filtering: Ensures the detected PlayerState belongs to the specific Controller.
 * - RAII: Automatically cleans up the delegate handle when this binder is destroyed.
 */
class FGCFPlayerReadyStateBinder final : public FGCFContextBinder
{
public:
	/**
	 * Creates and activates a binder for the specified Controller.
	 *
	 * @param InGFCM        The GameFrameworkComponentManager instance.
	 * @param InController  The controller that owns (or will own) the PlayerState.
	 * @param InDelegate    Callback executed when the PlayerState becomes ready.
	 * @param bIsAutoActivate If true, immediately starts the resolution process.
	 */
	static TUniquePtr<FGCFPlayerReadyStateBinder> CreateBinder(UGameFrameworkComponentManager* InGFCM, AController* InController, FGCFOnPlayerReadyStateChangedNative::FDelegate&& InDelegate, bool bIsAutoActivate = true)
	{
		TUniquePtr<FGCFPlayerReadyStateBinder> Binder(new FGCFPlayerReadyStateBinder(InGFCM, InController, MoveTemp(InDelegate)));
		if (bIsAutoActivate) {
			Binder->Activate();
		}
		return Binder;
	}

private:
	FGCFPlayerReadyStateBinder(UGameFrameworkComponentManager* InGFCM, AController* InController, FGCFOnPlayerReadyStateChangedNative::FDelegate&& InDelegate);

	/**
	 * Resets the tracker handle, unregistering the delegate from the PlayerState.
	 */
	void Deactivate() override;

	/**
	 * Checks if the Controller already has a valid PlayerState assigned.
	 * @return true if PlayerState exists and binding was successful.
	 */
	virtual bool TryResolveImmediate() override;

	/**
	 * Checks if a newly initialized PlayerState belongs to the tracked Controller.
	 */
	virtual bool TryResolveEvent(AActor * Actor, FName EventName) override;

private:
	/** The controller instance to monitor. */
	TWeakObjectPtr<AController> Controller;

	/** Handle for the scoped binding to the PlayerState. */
	TUniquePtr<FGCFDelegateHandle> TrackerHandle;

	/** The user callback. */
	FGCFOnPlayerReadyStateChangedNative::FDelegate Delegate;
};
