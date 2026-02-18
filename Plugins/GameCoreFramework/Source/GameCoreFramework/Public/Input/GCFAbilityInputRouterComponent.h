// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"

#include "GCFShared.h"
#include "Components/ControllerComponent.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "System/Binder/GCFContextBinder.h"
#include "GCFAbilityInputRouterComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UGCFAbilitySystemComponent;

/**
 * @brief Centralized router that dispatches input events to the appropriate Ability System Component (ASC).
 *
 * This component resides on the PlayerController and acts as the bridge between
 * the Input System (Physical Input) and the Gameplay Ability System (Logical Action).
 *
 * [Routing Logic]
 * It inspects the GameplayTag associated with the input and routes it based on the namespace:
 * - "Ability.Input.Player.*" -> Routes to the PlayerState's ASC (Soul Context).
 * - "Ability.Input.Pawn.*"   -> Routes to the currently possessed Pawn's ASC (Body Context).
 *
 * [Benefit]
 * This decoupling allows the Controller to trigger abilities on the Pawn without
 * tightly coupling the input logic to a specific Pawn class.
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = PlayerController, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision, Networking, Replication), meta = (BlueprintSpawnableComponent, CollapseCategories))
class UGCFAbilityInputRouterComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UE_API UGCFAbilityInputRouterComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * Routes an input tag to the correct Ability System Component based on its hierarchy.
	 *
	 * @param InputTag The tag identifying the input action (e.g., "Ability.Input.Pawn.Jump").
	 * @param bPressed True if the key was pressed, False if released.
	 */
	UE_API void RouteInputTag(const FGameplayTag& InputTag, bool bPressed);

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/**
	 * Updates the reference to the Pawn's ASC when possession changes.
	 */
	void HandlePossessedPawnChanged(AActor* Actor, bool bPossessed);

	/**
	 * Updates the reference to the PlayerState's ASC when the player becomes ready.
	 */
	void HandlePlayerReadyStateChanged(const FGCFPlayerReadyStateSnapshot& Snapshot);


	bool HasMachingAbility(UAbilitySystemComponent* ASC, const FGameplayTag& InputTag) const;


	void ExecRouteTag(UGCFAbilitySystemComponent* ASC, const FGameplayTag& InputTag, bool bPressed) const;

private:
	/** Reference to the "Soul" ASC (Persistent, e.g., Global Skills, Meta-game). */
	TWeakObjectPtr<UGCFAbilitySystemComponent> PlayerStateASC;

	/** Reference to the "Body" ASC (Transient, e.g., Movement, Combat). */
	TWeakObjectPtr<UGCFAbilitySystemComponent> CurrentPawnASC;

	/** Lifecycle management for event binders. */
	TArray<TUniquePtr<FGCFContextBinder>> BinderList;

	EGCFPlayerReadyState CachedPlayerReadyState;
};

#undef UE_API