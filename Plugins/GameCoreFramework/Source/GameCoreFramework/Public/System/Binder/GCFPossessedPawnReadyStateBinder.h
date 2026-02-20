// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "System/Binder/GCFContextBinder.h"
#include "System/Lifecycle/GCFStateTypes.h"

class UGameFrameworkComponentManager;
class FGCFDelegateHandle;
class AController;

/**
 * A dynamic binder that tracks the "Ready State" of whatever Pawn is currently possessed by a specific Controller.
 *
 * Mechanism (Chain Reaction):
 * 1. Monitors the Controller for possession events (Possessed / Unpossessed).
 * 2. When a Pawn is possessed, it automatically creates a scoped binding to that Pawn's Ready State.
 * 3. When unpossessed (or swapped), it releases the old binding and waits for the new one.
 *
 * Use Case:
 * - Essential for HUDs or Input Systems that need to initialize ONLY when the "Body" (Pawn) is fully ready,
 * but must survive through respawns or vehicle transitions.
 */
class FGCFPossessedPawnReadyStateBinder final : public FGCFContextBinder
{
public:
	/**
	 * Creates a binder that persists across pawn possession changes.
	 *
	 * @param InGFCM        The GameFrameworkComponentManager instance.
	 * @param InController  The controller to monitor.
	 * @param InDelegate    The callback executed when the CURRENT pawn's state changes.
	 * @param bAutoActivate If true, starts monitoring immediately.
	 */
	static TUniquePtr<FGCFPossessedPawnReadyStateBinder> CreateBinder(UGameFrameworkComponentManager* InGFCM, AController* InController, FGCFOnPawnReadyStateChangedNative::FDelegate&& InDelegate, bool bAutoActivate = true)
	{
		TUniquePtr<FGCFPossessedPawnReadyStateBinder> Binder (new FGCFPossessedPawnReadyStateBinder(InGFCM, InController, MoveTemp(InDelegate)));
		if (bAutoActivate) {
			Binder->Activate();
		}
		return Binder;
	}
private:
	FGCFPossessedPawnReadyStateBinder(UGameFrameworkComponentManager* InGFCM, AController* InController, FGCFOnPawnReadyStateChangedNative::FDelegate&& InDelegate);

	/** Resets the tracker handle. */
	void Deactivate() override;

	/**
	 * Checks if the controller already possesses a pawn and binds to it.
	 * Returns false to ensure the event listener remains active for future pawn swaps.
	 */
	virtual bool TryResolveImmediate() override;

	/**
	 * Handles dynamic possession/unpossession events to swap the internal binding.
	 */
	virtual bool TryResolveEvent(AActor* Actor, FName EventName) override;

private:
	TWeakObjectPtr<AController> Controller;
	TWeakObjectPtr<APawn> Pawn;

	/** RAII handle for the currently possessed pawn's ready state delegate. */
	TUniquePtr<FGCFDelegateHandle> TrackerHandle;

	FGCFOnPawnReadyStateChangedNative::FDelegate Delegate;
};
