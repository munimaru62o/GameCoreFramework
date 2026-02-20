// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "System/Lifecycle/GCFStateTypes.h"
#include "Components/PawnComponent.h"
#include "System/Composer/GCFGenericStateComposer.h" // デストラクタの中身が見えている必要がある
#include "GCFPawnReadyStateComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class FGCFGenericStateComposer;
class FGCFContextBinder;
struct FActorInitStateChangedParams;

/**
 * @brief Component responsible for aggregating and managing the initialization state of a Pawn.
 *
 * This component hooks into the GameFrameworkComponentManager (GFCM) extension events
 * and maintains the current readiness state as a bitmask flag.
 *
 * [Design Philosophy]
 * This component provides an "Abstraction of Time" for actor initialization.
 * In a multiplayer environment, the initialization order of sub-systems (Input, AbilitySystem, Possession)
 * is often non-deterministic due to network latency or replication timing.
 *
 * By using this component, consumers can write initialization logic in a consistent flow
 * without worrying about "whether it is already ready" or "when it will be ready".
 *
 * It serves as the "Single Source of Truth" for the Pawn's readiness state within the Controller context.
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = Pawn, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision), meta = (BlueprintSpawnableComponent))
class UGCFPawnReadyStateComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UGCFPawnReadyStateComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * Static helper to retrieve the ReadyState component from an actor.
	 * @return The component instance, or nullptr if not found.
	 */
	UFUNCTION(BlueprintPure, Category = "GCF|ReadyState")
	static UGCFPawnReadyStateComponent* FindGCFPawnReadyStateComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UGCFPawnReadyStateComponent>() : nullptr); }

	/**
	 * Registers a delegate to observe state changes, with an option to execute immediately.
	 *
	 * [Use Case]
	 * Simplifies the pattern: "If already ready, run now. If not, wait until ready."
	 *
	 * @param Delegate           The callback delegate (created via FDelegate::CreateUObject, etc.).
	 * @param bExecuteImmediately If true, the delegate is executed immediately with the current state snapshot.
	 * Recommended (true) to prevent race conditions where the state becomes valid before registration.
	 * @return FDelegateHandle   A handle used to unregister the delegate via RemoveDelegate().
	 */
	UE_API FDelegateHandle RegisterAndExecuteDelegate(const FGCFOnPawnReadyStateChangedNative::FDelegate& Delegate, bool bExecuteImmediately = true);

	/**
	 * Unregisters a previously registered delegate.
	 * @param Handle The handle returned by RegisterAndExecuteDelegate().
	 */
	UE_API void RemoveDelegate(const FDelegateHandle& Handle);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void HandleOnActorInitStateChanged(const FActorInitStateChangedParams& Params);

	void Reevaluate();
	void EnsureBinderBuilt();

	FGCFPawnReadyStateSnapshot MakeSnapshot(EGCFPawnReadyState State);

protected:
	/** Blueprint-assignable delegate for state changes. */
	UPROPERTY(BlueprintAssignable, Category = "GCF|ReadyState")
	FGCFOnPawnReadyStateChangedBP OnReadyStateChangedBP;

private:
	/** Native multicast delegate for state changes. */
	FGCFOnPawnReadyStateChangedNative OnReadyStateChangedNative;

	/** Logic processor that evaluates predicates to determine the current state bitmask. */
	TUniquePtr<FGCFGenericStateComposer> Composer;

	/** The last evaluated state, cached to detect transitions. */
	EGCFPawnReadyState CachedState = EGCFPawnReadyState::None;
};


#undef UE_API