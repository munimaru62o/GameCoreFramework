// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/Lifecycle/GCFPlayerExtensionComponent.h"

#include "GCFShared.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "System/Lifecycle/GCFStateFunctionLibrary.h"
#include "System/Lifecycle/GCFGameFeatureFunctionLibrary.h"
#include "AbilitySystem/GCFAbilitySystemFunctionLibrary.h"
#include "Components/GameFrameworkComponentManager.h"
#include "AbilitySystem/GCFAbilitySystemComponent.h"
#include "System/Binder/GCFControllerPossessionBinder.h"


UGCFPlayerExtensionComponent::UGCFPlayerExtensionComponent(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UGCFPlayerExtensionComponent::OnRegister()
{
	Super::OnRegister();
	RegisterInitStateFeature();
}


void UGCFPlayerExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for changes to all features
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// Notifies state manager that we have spawned, then try rest of default initialization
	ensure(TryToChangeInitState(GCFGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}


void UGCFPlayerExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeAbilitySystem();
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}


void UGCFPlayerExtensionComponent::InitializeAbilitySystem(UGCFAbilitySystemComponent* InASC, APawn* InAvatarActor)
{
	check(InASC);

	APlayerState* PlayerState = GetPlayerState<APlayerState>();
	if (!PlayerState) {
		return;
	}

	if (AbilitySystemComponent == InASC) {
		return;
	}

	// 1. Clean up old ASC if exists (Safety measure)
	if (AbilitySystemComponent) {
		UninitializeAbilitySystem();
	}

	// 2. Bind new ASC
	AbilitySystemComponent = InASC;

	// 3. Update Actor Info (Link Soul and Body)
	// Note: Pawn might be nullptr here (Spectator, etc), which is valid for GAS Init.
	AbilitySystemComponent->InitAbilityActorInfo(PlayerState, InAvatarActor);

	// 4. Apply data-driven settings (Future implementation)
	// if (ensure(PawnData)) { ... }

	UE_LOG(LogGCFPlayer, Log, TEXT("ASC Initialized for [%s]: Owner=[%s], Avatar=[%s]"),
		   *GetNameSafe(PlayerState), *GetNameSafe(PlayerState), *GetNameSafe(InAvatarActor));

	OnAbilitySystemInitialized.Broadcast();
}


void UGCFPlayerExtensionComponent::UninitializeAbilitySystem()
{
	if (!AbilitySystemComponent) {
		return;
	}

	FGameplayTagContainer AbilityTypesToIgnore;
	AbilityTypesToIgnore.AddTag(GCFGameplayTags::Ability_Behavior_SurvivesDeath);

	AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
	AbilitySystemComponent->ClearAbilityInput();
	AbilitySystemComponent->RemoveAllGameplayCues();
	AbilitySystemComponent->ClearActorInfo();

	OnAbilitySystemUninitialized.Broadcast();
	AbilitySystemComponent = nullptr;
}


void UGCFPlayerExtensionComponent::RefreshAbilityActorInfo(APawn* Pawn)
{
	APlayerState* PlayerState = GetPlayerState<APlayerState>();
	if (!PlayerState) {
		return;
	}

	if (AbilitySystemComponent) {
		// Re-link the ASC to the new Pawn (Avatar)
		AbilitySystemComponent->InitAbilityActorInfo(PlayerState, Pawn);

		UE_LOG(LogGCFPlayer, Log, TEXT("GAS ActorInfo Refreshed for [%s]: Owner=[%s], Avatar=[%s]"),
			   *GetNameSafe(Pawn), *GetNameSafe(PlayerState), *GetNameSafe(Pawn));
	}
}


void UGCFPlayerExtensionComponent::CheckDefaultInitialization()
{
	if (bIsCheckingDefaultInitialization) {
		return;
	}

	TGuardValue<bool> RecursionGuard(bIsCheckingDefaultInitialization, true);

	// Before checking our progress, try progressing any other features we might depend on
	CheckDefaultInitializationForImplementers();

	static const TArray<FGameplayTag> StateChain = {
		GCFGameplayTags::InitState_Spawned,
		GCFGameplayTags::InitState_DataAvailable,
		GCFGameplayTags::InitState_DataInitialized,
		GCFGameplayTags::InitState_GameplayReady
	};

	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}


bool UGCFPlayerExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APlayerState* PlayerState = GetPlayerState<APlayerState>();

	if (!CurrentState.IsValid() && DesiredState == GCFGameplayTags::InitState_Spawned) {
		if (PlayerState) {
			return true;
		}
	} else if (CurrentState == GCFGameplayTags::InitState_Spawned && DesiredState == GCFGameplayTags::InitState_DataAvailable) {
		if (const APlayerController* Controller = PlayerState->GetPlayerController()) {
			// 1. If not Local PC (Server/Other Client), just having Controller is enough.
			// 2. If Local PC, wait for GetLocalPlayer() to be valid.
			if (!Controller->IsLocalPlayerController() || Controller->GetLocalPlayer()) {
				return true;
			}
		}
	} else if (CurrentState == GCFGameplayTags::InitState_DataAvailable && DesiredState == GCFGameplayTags::InitState_DataInitialized) {
		return true;
	} else if (CurrentState == GCFGameplayTags::InitState_DataInitialized && DesiredState == GCFGameplayTags::InitState_GameplayReady) {
		return true;
	}
	return false;
}


void UGCFPlayerExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	APlayerState* PlayerState = GetPlayerState<APlayerState>();

	if (DesiredState == GCFGameplayTags::InitState_Spawned) {
		HandleSpawned(Manager, PlayerState);
	}
	if (DesiredState == GCFGameplayTags::InitState_DataAvailable) {
		HandleDataAvailable(Manager, PlayerState);
	}
	if (DesiredState == GCFGameplayTags::InitState_DataInitialized) {
		HandleDataInitialized(Manager, PlayerState);
	}
	if (DesiredState == GCFGameplayTags::InitState_GameplayReady) {
		if (Manager) {
			Manager->SendGameFrameworkComponentExtensionEvent(PlayerState, GCF::Names::Event_Player_Ready_GamePlay);
		}
	}
}


void UGCFPlayerExtensionComponent::HandleSpawned(UGameFrameworkComponentManager* Manager, APlayerState* PlayerState)
{
	UGCFGameFeatureFunctionLibrary::ReadyFeature(Manager, this, PlayerState, GCF::Names::Feature_Player_PlayerState);
	PlayerState->OnPawnSet.AddDynamic(this, &UGCFPlayerExtensionComponent::HandleOnPawnSet);
}


void UGCFPlayerExtensionComponent::HandleDataAvailable(UGameFrameworkComponentManager* Manager, APlayerState* PlayerState)
{
	// Notify Controller Ready
	UGCFGameFeatureFunctionLibrary::ReadyFeature(Manager, this, PlayerState, GCF::Names::Feature_Player_Controller);

	// Start listening for possession changes
	PossessionHandle = UGCFStateFunctionLibrary::BindPossessionScoped(
		PlayerState->GetOwningController(), FOnPossessedPawnChangedNative::FDelegate::CreateUObject(this, &ThisClass::HandlePossessedPawnChanged)
	);
}


void UGCFPlayerExtensionComponent::HandleDataInitialized(UGameFrameworkComponentManager* Manager, APlayerState* PlayerState)
{
	// Initialize AbilitySystem (Avatar might be null here, which is fine)
	if (UGCFAbilitySystemComponent* ASC = UGCFAbilitySystemFunctionLibrary::GetPlayerStateAbilitySystemComponent<UGCFAbilitySystemComponent>(PlayerState)) {
		InitializeAbilitySystem(ASC, PlayerState->GetPawn());
		UGCFGameFeatureFunctionLibrary::ReadyFeature(Manager, this, PlayerState, GCF::Names::Feature_Player_AbilitySystem);
	}

	// Notify Input Ready
	if (Manager) {
		Manager->SendGameFrameworkComponentExtensionEvent(PlayerState, GCF::Names::Event_Player_Ready_Input);
	}
}


void UGCFPlayerExtensionComponent::NotifyOwningControllerChanged()
{
	CheckDefaultInitialization();
}


void UGCFPlayerExtensionComponent::HandleOnPawnSet(APlayerState* Player, APawn* NewPawn, APawn* OldPawn)
{
	// Update GAS Avatar when Pawn is set
	RefreshAbilityActorInfo(NewPawn);
	CheckDefaultInitialization();
}


void UGCFPlayerExtensionComponent::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	// Redundant safety check to ensure Avatar is updated
	RefreshAbilityActorInfo(NewPawn);
	CheckDefaultInitialization();
}


void UGCFPlayerExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName != GetFeatureName()) {
		if (Params.FeatureState == GCFGameplayTags::InitState_DataAvailable) {
			CheckDefaultInitialization();
		}
	}
}


void UGCFPlayerExtensionComponent::OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject())) {
		OnAbilitySystemInitialized.Add(Delegate);
	}

	if (AbilitySystemComponent) {
		Delegate.Execute();
	}
}


void UGCFPlayerExtensionComponent::OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject())) {
		OnAbilitySystemUninitialized.Add(Delegate);
	}
}