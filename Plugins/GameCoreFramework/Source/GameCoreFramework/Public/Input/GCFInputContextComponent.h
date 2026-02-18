// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "System/Lifecycle/GCFStateTypes.h"
#include "Input/GCFInputTypes.h"
#include "GCFInputContextComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API


class FGCFContextBinder;

/**
 * @brief Component that acts as the "Gatekeeper" for the Input System.
 *
 * It resides on the Controller and aggregates the readiness states of both the "Soul" (PlayerState)
 * and the "Body" (Pawn).
 *
 * [Responsibility]
 * - Monitors the initialization lifecycle of the Player and the possessed Pawn.
 * - Determines if input processing should be allowed based on aggregated states.
 * - Broadcasts events when the input context availability changes (Allowed <-> Blocked).
 *
 * [Why this is needed]
 * Prevents "Input before Initialization" issues, such as trying to move a Pawn
 * before its movement component or ability system is fully replicated and initialized.
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = PlayerController, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision, Networking, Replication), meta = (BlueprintSpawnableComponent, CollapseCategories))
class UGCFInputContextComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UE_API UGCFInputContextComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * Registers a delegate to be notified when the input allowance state changes.
	 *
	 * @param Delegate            The callback to register.
	 * @param bExecuteImmediately If true, immediately executes the callback with the current state.
	 * Crucial for UI or logic that attaches after initialization has already occurred.
	 * @return FDelegateHandle    Handle for unregistering.
	 */
	UE_API FDelegateHandle RegisterAndExecuteDelegate(const FOnInputContextEvaluatedNative::FDelegate& Delegate, bool bExecuteImmediately = true);

	/** Unregisters the delegate. */
	UE_API void RemoveDelegate(const FDelegateHandle& Handle);

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	/**
	 * Handler invoked when the possessed Pawn's readiness snapshot changes.
	 * Updates the internal state bitmask regarding the "Body".
	 */
	void HandlePawnReadyStateChanged(const FGCFPawnReadyStateSnapshot& Snapshot);
	/**
	 * Handler invoked when the PlayerState's readiness snapshot changes.
	 * Updates the internal state bitmask regarding the "Soul".
	 */
	void HandlePlayerReadyStateChanged(const FGCFPlayerReadyStateSnapshot& Snapshot);

	/** Updates a specific bit in the context state and triggers evaluation. */
	void UpdateState(EGCFInputContextState StateBit, bool bEnable);

	/** Checks the aggregated state and broadcasts changes if the "Input Allowed" status flips. */
	void Evaluate();

	/** @return True if all required conditions (Player Ready & Pawn Ready) are met. */
	bool IsInputAllowed() const;

private:
	/** Broadcasts (CurrentStateBits, bIsInputAllowed). */
	FOnInputContextEvaluatedNative OnInputContextEvaluatedNative;

	/** List of active binders (Observers) for Player and Pawn states. */
	TArray<TUniquePtr<FGCFContextBinder>> BinderList;

	EGCFPlayerReadyState CachedPlayerReadyState = EGCFPlayerReadyState::None;
	EGCFPawnReadyState CachedPawnReadyState = EGCFPawnReadyState::None;
	EGCFInputContextState CurrentContextState = EGCFInputContextState::None;

	bool bLastEvaluatedInputEnabled = false;
};


#undef UE_API