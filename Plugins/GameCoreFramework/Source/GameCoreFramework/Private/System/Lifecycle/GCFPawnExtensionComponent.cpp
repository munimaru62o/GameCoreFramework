// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#include "System/Lifecycle/GCFPawnExtensionComponent.h"

#include "GCFShared.h"
#include "Actor/Data/GCFPawnData.h"
#include "AbilitySystem/GCFAbilitySystemComponent.h"
#include "AbilitySystem/GCFAbilityTagRelationshipMapping.h"
#include "AbilitySystem/GCFAbilitySystemFunctionLibrary.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Camera/GCFCameraComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Player/GCFPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Input/GCFPawnInputBridgeComponent.h"
#include "Input/GCFInputFunctionLibrary.h"
#include "System/Lifecycle/GCFGameFeatureFunctionLibrary.h"
#include "Movement/GCFMovementFunctionLibrary.h"
#include "Movement/GCFMovementConfigReceiver.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFPawnExtensionComponent)

class FLifetimeProperty;
class UActorComponent;


UGCFPawnExtensionComponent::UGCFPawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	PawnData = nullptr;
	AbilitySystemComponent = nullptr;
}

void UGCFPawnExtensionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGCFPawnExtensionComponent, PawnData);
}

void UGCFPawnExtensionComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetPawn<APawn>();
	TArray<UActorComponent*> GCFPawnExtensionComponents;
	Pawn->GetComponents(UGCFPawnExtensionComponent::StaticClass(), GCFPawnExtensionComponents);
	ensureAlwaysMsgf((GCFPawnExtensionComponents.Num() == 1), TEXT("Only one GCFPawnExtensionComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// Register with the init state system early, this will only work if this is a game world
	RegisterInitStateFeature();
}

void UGCFPawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Start listening for state changes
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// Advance state: Spawned
	ensure(TryToChangeInitState(GCFGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UGCFPawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeAbilitySystem();
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

void UGCFPawnExtensionComponent::SetPawnData(const UGCFPawnData* InPawnData)
{
	check(InPawnData);

	APawn* Pawn = GetPawnChecked<APawn>();
	if (Pawn->GetLocalRole() != ROLE_Authority) {
		return;
	}

	if (PawnData) {
		UE_LOG(LogGCFCharacter, Error, TEXT("Trying to set PawnData [%s] on pawn [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(Pawn), *GetNameSafe(PawnData));
		return;
	}

	PawnData = InPawnData;
	Pawn->ForceNetUpdate();

	CheckDefaultInitialization();
}


void UGCFPawnExtensionComponent::OnRep_PawnData()
{
	CheckDefaultInitialization();
}


void UGCFPawnExtensionComponent::InitializeAbilitySystem(UGCFAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	check(InASC);
	check(InOwnerActor);

	if (AbilitySystemComponent == InASC) {
		// Just refresh info if it's the same ASC (e.g., reconnecting)
		RefreshAbilityActorInfo();
		return;
	}

	// 1. Cleanup old ASC if exists
	if (AbilitySystemComponent) {
		UninitializeAbilitySystem();
	}

	// 2. Bind new ASC
	AbilitySystemComponent = InASC;

	// 3. Update Actor Info (Link Soul and Body)
	APawn* Pawn = GetPawnChecked<APawn>();
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

	// 4. Apply configuration from PawnData (e.g., Block/Cancel tags)
	if (ensure(PawnData)) {
		AbilitySystemComponent->SetTagRelationshipMapping(PawnData->TagRelationshipMapping);
	}

	UE_LOG(LogGCFCharacter, Log, TEXT("ASC Initialized for [%s]: Owner=[%s], Avatar=[%s]"),
		   *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(Pawn));

	OnAbilitySystemInitialized.Broadcast();
}


void UGCFPawnExtensionComponent::UninitializeAbilitySystem()
{
	if (!AbilitySystemComponent) {
		return;
	}

	// 1. Cleanup Abilities granted by this Pawn
	// This removes weapons/skills that are tied to this specific body.
	if (AbilitySystemComponent->IsOwnerActorAuthoritative()) {
		AbilitySetHandles.TakeFromAbilitySystem(AbilitySystemComponent);
	}

	// 2. Stop ongoing effects/anims tied to this pawn
	FGameplayTagContainer AbilityTypesToIgnore;
	AbilityTypesToIgnore.AddTag(GCFGameplayTags::Ability_Behavior_SurvivesDeath);

	AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
	AbilitySystemComponent->ClearAbilityInput();
	AbilitySystemComponent->RemoveAllGameplayCues();

	// 3. Detach Avatar if we are still the avatar
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner()) {
		AbilitySystemComponent->SetAvatarActor(nullptr);
	}

	OnAbilitySystemUninitialized.Broadcast();
	AbilitySystemComponent = nullptr;
}


void UGCFPawnExtensionComponent::HandleControllerChanged()
{
	RefreshAbilityActorInfo();
	CheckDefaultInitialization();
}


void UGCFPawnExtensionComponent::OnControllerAssigned()
{
	// Notify Controller Assignment
	if (APawn* Pawn = GetPawn<APawn>()) {
		if (UGameFrameworkComponentManager* Manager = GetComponentManager()) {
			Manager->SendGameFrameworkComponentExtensionEvent(Pawn, GCF::Names::Event_Pawn_ControllerAssigned);
		}
	}

	HandleControllerChanged();
}


void UGCFPawnExtensionComponent::HandlePlayerStateReplicated()
{
	RefreshAbilityActorInfo();
	CheckDefaultInitialization();
}

void UGCFPawnExtensionComponent::SetupPlayerInputComponent()
{
	CheckDefaultInitialization();
}

void UGCFPawnExtensionComponent::CheckDefaultInitialization()
{
	// Prevent recursion
	if (bIsCheckingDefaultInitialization) {
		return;
	}

	TGuardValue<bool> RecursionGuard(bIsCheckingDefaultInitialization, true);

	CheckDefaultInitializationForImplementers();

	static const TArray<FGameplayTag> StateChain = { 
		GCFGameplayTags::InitState_Spawned,
		GCFGameplayTags::InitState_DataAvailable,
		GCFGameplayTags::InitState_DataInitialized, 
		GCFGameplayTags::InitState_GameplayReady 
	};

	ContinueInitStateChain(StateChain);

}

bool UGCFPawnExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();
	if (!CurrentState.IsValid() && DesiredState == GCFGameplayTags::InitState_Spawned) {
		if (Pawn) {
			return true;
		}
	} else if (CurrentState == GCFGameplayTags::InitState_Spawned && DesiredState == GCFGameplayTags::InitState_DataAvailable) {
		// Must have PawnData to proceed
		if (PawnData) {
			return true;
		}
	} else if (CurrentState == GCFGameplayTags::InitState_DataAvailable && DesiredState == GCFGameplayTags::InitState_DataInitialized) {
		// Transition to Initialized is usually automatic once DataAvailable logic (HandleDataInitialized) completes
		return true;

	} else if (CurrentState == GCFGameplayTags::InitState_DataInitialized && DesiredState == GCFGameplayTags::InitState_GameplayReady) {
		return true;
	}
	return false;
}


void UGCFPawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (DesiredState == GCFGameplayTags::InitState_DataInitialized) {
		HandleDataInitialized(Manager);
	}
	if (DesiredState == GCFGameplayTags::InitState_GameplayReady) {
		if (APawn* Pawn = GetPawn<APawn>()) {
			if (Manager) {
				Manager->SendGameFrameworkComponentExtensionEvent(Pawn, GCF::Names::Event_Pawn_Ready_GamePlay);
			}
		}
	}
}


void UGCFPawnExtensionComponent::HandleDataInitialized(UGameFrameworkComponentManager* Manager)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn || !PawnData) return;

	// Notify PawnData is ready
	UGCFGameFeatureFunctionLibrary::ReadyFeature(Manager, this, Pawn, GCF::Names::Feature_Pawn_PawnData);

	// Setup each domain
	SetupAbilitySystem(Pawn, Manager);
	SetupInputConfig(Pawn);
	SetupMovementConfig(Pawn);
}


void UGCFPawnExtensionComponent::SetupAbilitySystem(APawn* Pawn, UGameFrameworkComponentManager* Manager)
{
	UGCFAbilitySystemComponent* ASC = UGCFAbilitySystemFunctionLibrary::ResolveAbilitySystemComponent<UGCFAbilitySystemComponent>(Pawn);
	if (!ASC) return;

	AActor* OwnerActor = UGCFAbilitySystemFunctionLibrary::ResolveAbilitySystemComponentOwner(Pawn);
	InitializeAbilitySystem(ASC, OwnerActor);

	if (Pawn->HasAuthority()) {
		for (const TObjectPtr<UGCFAbilitySet>& AbilitySet : PawnData->AbilitySets) {
			if (AbilitySet) AbilitySet->GiveToAbilitySystem(ASC, &AbilitySetHandles, Pawn);
		}
	}
	UGCFGameFeatureFunctionLibrary::ReadyFeature(Manager, this, Pawn, GCF::Names::Feature_Pawn_Ability);
}


void UGCFPawnExtensionComponent::SetupInputConfig(APawn* Pawn)
{
	if (!PawnData->InputConfig) return;

	// Register Input Config (Connecting to the InputBindingManager)
	// This uses the Bridge Component we created earlier.
	if (TScriptInterface<IGCFInputConfigProvider> Provider = UGCFInputFunctionLibrary::ResolveInputConfigProvider(this, EGCFInputSourceType::Pawn)) {
		Provider->AddInputConfig(PawnData->InputConfig);
	}
}


void UGCFPawnExtensionComponent::SetupMovementConfig(APawn* Pawn)
{
	if (!PawnData->MovementConfig) return;

	if (TScriptInterface<IGCFMovementConfigReceiver> Receiver = UGCFMovementFunctionLibrary::ResolveMovementConfigReceiver(Pawn)) {
		IGCFMovementConfigReceiver::Execute_ApplyMovementConfig(Receiver.GetObject(), PawnData->MovementConfig);
	}
}


void UGCFPawnExtensionComponent::RefreshAbilityActorInfo()
{
	if (!AbilitySystemComponent) {
		return;
	}

	APawn* Pawn = GetPawnChecked<APawn>();
	AActor* NewOwner = UGCFAbilitySystemFunctionLibrary::ResolveAbilitySystemComponentOwner(Pawn);

	// If owner is missing (e.g. unpossessed), we might want to detach or wait.
	// For now, only refresh if we have a valid owner.
	if (NewOwner) {
		AbilitySystemComponent->InitAbilityActorInfo(NewOwner, Pawn);

		UE_LOG(LogGCFCharacter, Log, TEXT("GAS ActorInfo Refreshed for [%s]: Owner=[%s], Avatar=[%s]"),
			   *GetNameSafe(Pawn), *GetNameSafe(NewOwner), *GetNameSafe(Pawn));
	}
}


void UGCFPawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName != GetFeatureName())
	{
		if (Params.FeatureState == GCFGameplayTags::InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
}

void UGCFPawnExtensionComponent::OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemInitialized.Add(Delegate);
	}

	if (AbilitySystemComponent)
	{
		Delegate.Execute();
	}
}

void UGCFPawnExtensionComponent::OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemUninitialized.Add(Delegate);
	}
}