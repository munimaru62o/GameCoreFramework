// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "GCFShared.h"
#include "Components/PlayerStateComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "AbilitySystem/GCFAbilitySet.h"
#include "GCFPlayerExtensionComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class AController;
class APlayerState;
class FGCFDelegateHandle;

/**
 * @brief Manages the initialization lifecycle of the PlayerState ("Soul").
 *
 * Implements the GameFrameworkInitStateInterface to provide a deterministic initialization chain:
 * [Spawned] -> [DataAvailable] -> [DataInitialized] -> [GameplayReady]
 *
 * [Key Responsibilities]
 * - Ensures PlayerState data (e.g. UniqueId, Team) is replicated before Gameplay starts.
 * - Initializes the AbilitySystemComponent (ASC) on the PlayerState.
 * - Acts as a "hub" for other components to wait for the Player to be ready.
 *
 * [Difference from PawnExtension]
 * This component persists as long as the player is connected, whereas PawnExtension dies with the Pawn.
 * It does NOT require a Pawn to reach "GameplayReady" state (e.g. for Spectators).
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = PlayerState, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision, Networking, Replication), meta = (BlueprintSpawnableComponent, CollapseCategories))
class UGCFPlayerExtensionComponent : public UPlayerStateComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UGCFPlayerExtensionComponent(const FObjectInitializer& ObjectInitializer);

	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return GCF::Names::Feature_Player_Extension; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

	/** Helper to find this component on an Actor (usually PlayerState). */
	UFUNCTION(BlueprintPure, Category = "GCF|Player")
	static UGCFPlayerExtensionComponent* FindGCFPlayerExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UGCFPlayerExtensionComponent>() : nullptr); }

	/** Gets the cached ASC. */
	UFUNCTION(BlueprintPure, Category = "GCF|Player")
	UGCFAbilitySystemComponent* GetAbilitySystemComponent() const { return AbilitySystemComponent; }
	
	// ------------------------------------------------------------------------------------------------
	// Ability System Lifecycle
	// ------------------------------------------------------------------------------------------------

	/** Register callback for when ASC is initialized. Broadcasts immediately if already ready. */
	void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);

	/** Register callback for when ASC is uninitialized. */
	void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);

	/** Called by PlayerState/Pawn when the owning controller changes. Triggers state re-evaluation. */
	void NotifyOwningControllerChanged();

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** * Initializes the ASC.
	 * If Pawn is present, sets it as Avatar. If not, initializes as "Soul only".
	 */
	void InitializeAbilitySystem(UGCFAbilitySystemComponent* InASC, APawn* InAvatarActor);

	/** Cleans up ASC references. */
	void UninitializeAbilitySystem();

	void RefreshAbilityActorInfo(APawn* Pawn);

	UFUNCTION()
	void HandleOnPawnSet(APlayerState* Player, APawn* NewPawn, APawn* OldPawn);

	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	void HandleSpawned(UGameFrameworkComponentManager* Manager, APlayerState* PlayerState);
	void HandleDataAvailable(UGameFrameworkComponentManager* Manager, APlayerState* PlayerState);
	void HandleDataInitialized(UGameFrameworkComponentManager* Manager, APlayerState* PlayerState);

private:
	FSimpleMulticastDelegate OnAbilitySystemInitialized;
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;

	UPROPERTY(Transient)
	TObjectPtr<UGCFAbilitySystemComponent> AbilitySystemComponent;

	// Handles for granted abilities (e.g. default player skills).
	FGCFAbilitySet_GrantedHandles AbilitySetHandles;

	// Watcher for Controller possession changes.
	TUniquePtr<FGCFDelegateHandle> PossessionHandle;

	/** Guard flag to prevent recursive calls during state checks. */
	bool bIsCheckingDefaultInitialization = false;
};


#undef UE_API