// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "System/Binder/GCFContextBinder.h"
#include "System/Lifecycle/GCFStateTypes.h"

class UGameFrameworkComponentManager;
class FGCFDelegateHandle;

/**
 * A specialized binder that monitors the aggregate "Ready State" of a specific Pawn.
 *
 * Features:
 * - Scoped Binding: Utilizes an internal handle (TrackerHandle) to automatically unregister the delegate
 * from the Pawn's state component when this binder is destroyed or deactivated.
 * - Integration: connect specifically to UGCFPawnReadyStateComponent via the component function library.
 */
class FGCFPawnReadyStateBinder final : public FGCFContextBinder
{
public:
	/**
	 * Creates and activates a binder for the specified Pawn.
	 *
	 * @param InGFCM        The GameFrameworkComponentManager instance.
	 * @param InPawn        The target Pawn to monitor.
	 * @param InDelegate    The callback to execute when the ready state changes.
	 * @param bIsAutoActivate If true, immediately attempts to bind or starts listening for the Pawn.
	 */
	static TUniquePtr<FGCFPawnReadyStateBinder> CreateBinder(UGameFrameworkComponentManager* InGFCM, APawn* InPawn, FGCFOnPawnReadyStateChangedNative::FDelegate&& InDelegate, bool bIsAutoActivate = true)
	{
		TUniquePtr<FGCFPawnReadyStateBinder> Binder(new FGCFPawnReadyStateBinder(InGFCM, InPawn, MoveTemp(InDelegate)));
		if (bIsAutoActivate) {
			Binder->Activate();
		}
		return Binder;
	}

private:
	FGCFPawnReadyStateBinder(UGameFrameworkComponentManager* InGFCM, APawn* InPawn, FGCFOnPawnReadyStateChangedNative::FDelegate&& InDelegate);

	/**
	 * Resets the tracker handle, effectively unregistering the delegate from the Pawn's component.
	 */
	void Deactivate() override;

	/**
	 * Checks if the Pawn exists and attempts to bind the state delegate immediately.
	 * @return true if the Pawn was found and binding was attempted; false otherwise.
	 */
	virtual bool TryResolveImmediate() override;

	/**
	 * Checks if the event target matches the tracked Pawn and retries binding.
	 */
	virtual bool TryResolveEvent(AActor* Actor, FName EventName) override;

private:
	/** The target Pawn to monitor. */
	TWeakObjectPtr<APawn> Pawn;

	/**
	 * RAII handle for the bound delegate.
	 * When this handle is reset, the delegate is removed from the target component.
	 */
	TUniquePtr<FGCFDelegateHandle> TrackerHandle;

	/** The user-provided callback. */
	FGCFOnPawnReadyStateChangedNative::FDelegate Delegate;
};
