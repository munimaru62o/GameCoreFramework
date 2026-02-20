// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * @brief RAII (Resource Acquisition Is Initialization) handle for managing Delegate lifecycles.
 *
 * Automatically executes an unbind action (lambda) when this handle goes out of scope or is destroyed.
 * Useful for ensuring delegates are cleanly removed when a component or binder is destroyed.
 */
class FGCFDelegateHandle
{
public:
	/**
	 * @param InUnbindAction The function/lambda to execute upon destruction (e.g., Delegate.Remove(Handle)).
	 */
	FGCFDelegateHandle(TFunction<void()> InUnbindAction)
		: UnbindAction(InUnbindAction)
	{}

	~FGCFDelegateHandle()
	{
		if (UnbindAction) {
			UnbindAction();
		}
	}

	// Disable copying to ensure unique ownership of the cleanup responsibility.
	FGCFDelegateHandle(const FGCFDelegateHandle&) = delete;
	FGCFDelegateHandle& operator=(const FGCFDelegateHandle&) = delete;

	/**
	 * @brief Factory method to create a scoped handle for component delegates.
	 * * @tparam TrackerType Component class type (e.g. UGCFPawnReadyStateComponent)
	 * @tparam RegisterFnType Member function pointer type for Registration
	 * @tparam RemoveFnType Member function pointer type for Removal
	 * @tparam DelegateType The delegate type being bound
	 * * @param Tracker The component instance to bind to.
	 * @param bExecuteImmediately Whether to execute the delegate immediately upon binding.
	 * @param RegisterFn Pointer to the Register function (e.g. &UComponent::RegisterDelegate).
	 * @param RemoveFn Pointer to the Remove function (e.g. &UComponent::RemoveDelegate).
	 * @param Delegate The delegate payload to bind.
	 */
	template<typename TrackerType, typename RegisterFnType, typename RemoveFnType, typename DelegateType>
	static TUniquePtr<FGCFDelegateHandle> CreateScoped(TrackerType* Tracker, bool bExecuteImmediately, RegisterFnType RegisterFn, RemoveFnType RemoveFn, DelegateType&& Delegate)
	{
		if (!Tracker || !Delegate.IsBound()) {
			return nullptr;
		}

		// Call Register function
		FDelegateHandle RawHandle = (Tracker->*RegisterFn)(Forward<DelegateType>(Delegate), bExecuteImmediately);
		TWeakObjectPtr<TrackerType> TrackerPtr(Tracker);

		// Wrap the Unbind logic in a lambda
		return MakeUnique<FGCFDelegateHandle>([TrackerPtr, RawHandle, RemoveFn]() {
			if (TrackerPtr.IsValid()) {
				(TrackerPtr.Get()->*RemoveFn)(RawHandle);
			}
		});
	}

private:
	TFunction<void()> UnbindAction;
};


DECLARE_DELEGATE_TwoParams(FGCFBooleanStateSignature, AActor*, bool);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPossessedPawnChangedNative, APawn* /*OldPawn*/, APawn* /*NewPawn*/);