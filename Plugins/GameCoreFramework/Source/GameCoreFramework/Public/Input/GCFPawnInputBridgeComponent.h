// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"

#include "GCFShared.h"
#include "Components/PawnComponent.h"
#include "GameplayTagContainer.h"
#include "Input/GCFInputConfigProvider.h"
#include "System/Binder/GCFContextBinder.h"
#include "GCFPawnInputBridgeComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UGCFInputComponent;
class UGCFInputConfig;
class UGCFAbilityInputRouterComponent;
class IGCFPawnDataProvider;

/**
 * @brief Component attached to a Pawn that serves as the "Input Definition Source" for that Pawn.
 *
 * [Architecture: Bridge Pattern]
 * In this framework, the Controller handles the *processing* of input, but the Pawn holds the *definition*
 * of what inputs are available (e.g., Character-specific skills, Vehicle controls).
 *
 * This component bridges that gap by:
 * 1. Waiting for the Pawn to be fully initialized via ReadyState (Possessed + GameplayReady).
 * 2. Sending its InputConfig list to the Controller's BindingManager.
 * 3. Routing the resulting physical input events back to the Controller's AbilityInputRouter.
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = Pawn, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision, Networking, Replication), meta = (BlueprintSpawnableComponent, CollapseCategories))
class UGCFPawnInputBridgeComponent : public UPawnComponent, public IGCFInputConfigProvider
{
	GENERATED_BODY()

public:
	UE_API UGCFPawnInputBridgeComponent(const FObjectInitializer& ObjectInitializer);

	// ~IGCFInputConfigProvider Interface
	UE_API virtual EGCFInputSourceType GetInputSourceType() const override { return EGCFInputSourceType::Pawn; };
	UE_API virtual void AddInputConfig(const UGCFInputConfig* InputConfig) override;
	UE_API virtual void RemoveInputConfig(const UGCFInputConfig* Config) override;
	UE_API virtual TArray<const UGCFInputConfig*> GetInputConfigList() const override { return InputConfigList; };
	// ~End IGCFInputConfigProvider Interface

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/**
	 * Callback invoked by the InputBindingManager to actually apply the bindings.
	 * Binds Input Actions to this component's handler functions using the provided config.
	 */
	TArray<FGCFBindingReceipt> HandleInputBinding(UGCFInputComponent* InputComponent, TScriptInterface<IGCFInputConfigProvider> Provider);

	/**
	 * Monitors the Pawn's readiness state.
	 * Registers input bindings only when the Pawn is fully "Ready" (Possessed + Gameplay Initialized).
	 */
	void HandlePawnReadyStateChanged(const FGCFPawnReadyStateSnapshot& Snapshot);

	/** Routes the "Pressed" event to the Ability System via the Controller's Router. */
	void HandleInputPressed(FGameplayTag InputTag);

	/** Routes the "Released" event. */
	void HandleInputReleased(FGameplayTag InputTag);

private:
	/** Cached reference to the Router component on the Controller (The "Soul"). */
	TWeakObjectPtr<UGCFAbilityInputRouterComponent> AbilityInputRouter;

	/** List of input configs currently active for this pawn (e.g., DefaultWeapon, CharacterSkills). */
	TArray<TObjectPtr<const UGCFInputConfig>> InputConfigList;

	/** RAII handle for managing the lifecycle of the ReadyState observer. */
	TUniquePtr<FGCFContextBinder> PawnReadyBinder;

	EGCFPawnReadyState CachedPawnReadyState = EGCFPawnReadyState::None;
};


#undef UE_API