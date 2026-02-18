// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"

#include "GCFShared.h"
#include "Components/ControllerComponent.h"
#include "GameplayTagContainer.h"
#include "Input/GCFInputConfigProvider.h"
#include "GCFPlayerInputBridgeComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UInputComponent;
class UGCFInputConfig;
class UGCFAbilityInputRouterComponent;

/**
 * @brief Component attached to the PlayerController to handle "Soul-level" inputs.
 *
 * [Responsibility]
 * - Converts Input Actions (IA) into Gameplay Tags.
 * - Manages inputs that must persist regardless of the Pawn's state (e.g., UI, Chat, System Menus).
 * - Unlike UGCFPawnInputBridgeComponent, this component does NOT depend on a specific Pawn class.
 *
 * [Architecture]
 * It implements IGCFInputConfigProvider to expose Controller-specific InputConfigs
 * to the central InputBindingManager.
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = PlayerController, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision, Networking, Replication), meta = (BlueprintSpawnableComponent, CollapseCategories))
class UGCFPlayerInputBridgeComponent : public UControllerComponent, public IGCFInputConfigProvider
{
	GENERATED_BODY()

public:
	UE_API UGCFPlayerInputBridgeComponent(const FObjectInitializer& ObjectInitializer);

	// ~IGCFInputConfigProvider Interface
	UE_API virtual EGCFInputSourceType GetInputSourceType() const override { return EGCFInputSourceType::Player; };
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
	 * Callback invoked by the BindingManager to apply input bindings to the Enhanced Input system.
	 */
	TArray<FGCFBindingReceipt> HandleInputBinding(UGCFInputComponent* InputComponent, TScriptInterface<IGCFInputConfigProvider> Provider);

	/** Routes the "Pressed" event to the Ability System. */
	void HandleInputPressed(const FGameplayTag InputTag);

	/** Routes the "Released" event. */
	void HandleInputReleased(const FGameplayTag InputTag);

private:
	/** Reference to the Router component used to dispatch tags. */
	TWeakObjectPtr<UGCFAbilityInputRouterComponent> AbilityInputRouter;

	/** List of persistent input configs (e.g., DefaultMenuControls). */
	TArray<TObjectPtr<const UGCFInputConfig>> InputConfigList;
};

#undef UE_API