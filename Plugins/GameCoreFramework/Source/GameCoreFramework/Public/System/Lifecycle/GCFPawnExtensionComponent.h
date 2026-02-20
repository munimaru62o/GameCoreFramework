// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "GCFShared.h"
#include "Components/PawnComponent.h"
#include "AbilitySystem/GCFAbilitySet.h"
#include "Actor/Data/GCFPawnDataProvider.h"
#include "GCFPawnExtensionComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API 

namespace EEndPlayReason { enum Type : int; }

class UGameFrameworkComponentManager;
class UGCFAbilitySystemComponent;
class UGCFPawnData;
class UObject;
class UGCFCameraMode;
class UGCFInputConfig;
struct FActorInitStateChangedParams;
struct FFrame;
struct FGameplayTag;


/**
 * @brief Manager component for Pawn lifecycle, Data Asset application, and System Integration.
 *
 * [Responsibilities]
 * 1. Data Management: Applies 'PawnData' (Abilities, InputConfig, etc.) to the Pawn.
 * 2. Lifecycle Synchronization: Implements InitStateInterface to ensure the Pawn is fully "Ready"
 * only when Controller, PlayerState, and Data are all available.
 * 3. System Bridge: acts as the primary link between the Pawn ("Body") and the AbilitySystem ("Soul").
 *
 * [Why needed?]
 * Decouples the Pawn class from specific game logic. A generic "Character" can become a "Hero", "Villain",
 * or "NPC" simply by swapping the PawnData and attaching this component.
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = Pawn, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision, Networking, Replication), meta = (BlueprintSpawnableComponent, CollapseCategories))
class UGCFPawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface, public IGCFPawnDataProvider
{
	GENERATED_BODY()

public:

	UE_API UGCFPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);

	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return GCF::Names::Feature_Pawn_Extension; }
	UE_API virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	UE_API virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	UE_API virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	UE_API virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

	/** Returns the pawn extension component if one exists on the specified actor. */
	UFUNCTION(BlueprintPure, Category = "GCF|Pawn")
	static UGCFPawnExtensionComponent* FindGCFPawnExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UGCFPawnExtensionComponent>() : nullptr); }

	/** Gets the current ability system component. */
	UFUNCTION(BlueprintPure, Category = "GCF|Pawn")
	UGCFAbilitySystemComponent* GetAbilitySystemComponent() const { return AbilitySystemComponent; }

	/** Sets the PawnData asset. This triggers the initialization chain. */
	UE_API void SetPawnData(const UGCFPawnData* InPawnData);

	/**
	 * Links the "Soul" (OwnerActor: PlayerState/Controller) to the "Body" (Avatar: This Pawn).
	 * Typically called when the Pawn is possessed or replicated.
	 */
	UE_API void InitializeAbilitySystem(UGCFAbilitySystemComponent* InASC, AActor* InOwnerActor);

	/** Unlinks the Body from the Soul and cleans up abilities granted by this Pawn. */
	UE_API void UninitializeAbilitySystem();

	/** Called when the Controller changes (Possess/Unpossess). Triggers a refresh of ActorInfo. */
	UE_API void HandleControllerChanged();

	/** Called when the Controller assigned. */
	UE_API void OnControllerAssigned();

	/** Called when the PlayerState is replicated (Client side). */
	UE_API void HandlePlayerStateReplicated();

	/** Called when input setup is required. */
	UE_API void SetupPlayerInputComponent();

	// ------------------------------------------------------------------------------------------------
	// Delegates
	// ------------------------------------------------------------------------------------------------

	/** Register with the OnAbilitySystemInitialized delegate. */
	UE_API void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);

	/** Register with the OnAbilitySystemUninitialized delegate. */
	UE_API void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);

protected:
	UE_API virtual void OnRegister() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	UE_API void OnRep_PawnData();

	// IGCFPawnDataProvider interface
	virtual const UGCFPawnData* GetPawnDataInternal() const override { return PawnData; };

private:
	/** Handles the transition to "DataInitialized" state (Applies Abilities, Input, etc.). */
	void HandleDataInitialized(UGameFrameworkComponentManager* Manager);

	void SetupAbilitySystem(APawn* Pawn, UGameFrameworkComponentManager* Manager);
	void SetupInputConfig(APawn* Pawn);
	void SetupMovementConfig(APawn* Pawn);

	/** Updates the ASC's ActorInfo (Owner/Avatar) without resetting the entire system. */
	void RefreshAbilityActorInfo();

private:
	FSimpleMulticastDelegate OnAbilitySystemInitialized;
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;

	/** The Data Asset defining this Pawn's specific capabilities. */
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_PawnData, Category = "GCF|Pawn")
	TObjectPtr<const UGCFPawnData> PawnData;

	UPROPERTY(Transient)
	TObjectPtr<UGCFAbilitySystemComponent> AbilitySystemComponent;

	// Handles for abilities granted by the PawnData. Used to clean them up on death/destruction.
	FGCFAbilitySet_GrantedHandles AbilitySetHandles;

	bool bIsCheckingDefaultInitialization = false;
};

#undef UE_API
