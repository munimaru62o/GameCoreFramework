// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "System/Lifecycle/GCFStateTypes.h"
#include "Components/PlayerStateComponent.h"
#include "System/Composer/GCFGenericStateComposer.h"
#include "GCFPlayerReadyStateComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class FGCFGenericStateComposer;
class FGCFContextBinder;
struct FActorInitStateChangedParams;

/**
 * @brief Component responsible for aggregating and managing the initialization state of a PlayerState.
 *
 * This component acts as the central registry for the player's lifecycle milestones,
 * tracking whether the Input System, Ability System, and Possession logic have fully initialized.
 *
 * [Design Philosophy]
 * Similar to the PawnReadyStateComponent, this provides an "Abstraction of Time" but for the persistent Player entity.
 * It resolves the ambiguity of "When is the player ready to join the game logic?" by observing
 * distributed features and synthesizing them into a single, authoritative 'Ready State'.
 *
 * This allows systems like Matchmaking, HUD initialization, and Spawn Logic to depend on a single
 * reliable source rather than checking scattered boolean flags across multiple classes.
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = PlayerState, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision, Networking, Replication), meta = (BlueprintSpawnableComponent, CollapseCategories))
class UGCFPlayerReadyStateComponent : public UPlayerStateComponent
{
	GENERATED_BODY()

public:
	UE_API UGCFPlayerReadyStateComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * Static helper to retrieve the PlayerReadyState component from an actor.
	 * @return The component instance, or nullptr if not found.
	 */
	UFUNCTION(BlueprintPure, Category = "GCF|ReadyState")
	static UGCFPlayerReadyStateComponent* FindGCFPlayerReadyStateComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UGCFPlayerReadyStateComponent>() : nullptr); }

	/**
	 * Registers a delegate to observe state changes, with an option to execute immediately.
	 *
	 * [Use Case]
	 * Enables concise initialization logic: "Execute this callback when the player is ready,
	 * or immediately if they are already ready."
	 *
	 * @param Delegate           The callback delegate (created via FDelegate::CreateUObject, etc.).
	 * @param bExecuteImmediately If true, the delegate is executed immediately with the current state snapshot.
	 * Recommended (true) to ensure no state transitions are missed due to race conditions.
	 * @return FDelegateHandle   A handle used to unregister the delegate via RemoveDelegate().
	 */
	UE_API FDelegateHandle RegisterAndExecuteDelegate(const FGCFOnPlayerReadyStateChangedNative::FDelegate& Delegate, bool bExecuteImmediately = true);

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

	FGCFPlayerReadyStateSnapshot MakeSnapshot(EGCFPlayerReadyState State);

protected:
	/** Blueprint-assignable delegate for state changes. */
	UPROPERTY(BlueprintAssignable, Category = "GCF|ReadyState")
	FGCFOnPlayerReadyStateChangedBP OnReadyStateChangedBP;

private:
	/** Native multicast delegate for state changes. */
	FGCFOnPlayerReadyStateChangedNative OnReadyStateChangedNative;

	/** Logic processor that evaluates predicates to determine the current state bitmask. */
	TUniquePtr<FGCFGenericStateComposer> Composer;

	/** The last evaluated state, cached to detect transitions. */
	EGCFPlayerReadyState CachedState = EGCFPlayerReadyState::None;
};


#undef UE_API