// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/Lifecycle/GCFPlayerReadyStateComponent.h"

#include "GCFShared.h"
#include "System/Binder/GCFControllerPossessionBinder.h"
#include "System/Predicate/GCFFeaturePredicate.h"
#include "System/Predicate/GCFGameplayTagPredicate.h"
#include "Components/GameFrameworkComponentManager.h"
#include "System/GCFDebugFunctionLibrary.h"


UGCFPlayerReadyStateComponent::UGCFPlayerReadyStateComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


FDelegateHandle UGCFPlayerReadyStateComponent::RegisterAndExecuteDelegate(const FGCFOnPlayerReadyStateChangedNative::FDelegate& Delegate, bool bExecuteImmediately)
{
	// Ensure lazy initialization of the state composer
	EnsureBinderBuilt();

	if (bExecuteImmediately) {
		Delegate.ExecuteIfBound(MakeSnapshot(CachedState));
	}
	return OnReadyStateChangedNative.Add(Delegate);
}


void UGCFPlayerReadyStateComponent::RemoveDelegate(const FDelegateHandle& Handle)
{
	OnReadyStateChangedNative.Remove(Handle);
}


void UGCFPlayerReadyStateComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureBinderBuilt();
}


void UGCFPlayerReadyStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Composer.Reset();
	Super::EndPlay(EndPlayReason);
}


void UGCFPlayerReadyStateComponent::Reevaluate()
{
	if (!Composer) {
		return;
	}

	const EGCFPlayerReadyState NewState = (EGCFPlayerReadyState)Composer->Compute();
	if (NewState != CachedState) {
		CachedState = NewState;
		OnReadyStateChangedBP.Broadcast(NewState);
		OnReadyStateChangedNative.Broadcast(MakeSnapshot(NewState));

		// Send debug message.
		if (APlayerState* PS = GetPlayerState<APlayerState>()) {
			if (APlayerController* Controller = PS->GetPlayerController()) {
				if (Controller->IsLocalController()) {
					UGCFDebugFunctionLibrary::SendPlayerStateBitMessage(this, EGCFDebugStateCategory::ReadyStatePlayer, NewState);
				}
			}
		}
	}
}


void UGCFPlayerReadyStateComponent::HandleOnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	Reevaluate();
}


void UGCFPlayerReadyStateComponent::EnsureBinderBuilt()
{
	if (Composer) {
		return;
	}

	APlayerState* PS = GetPlayerState<APlayerState>();
	if (!PS) {
		return;
	}

	UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(GetOwner());
	if (!GFCM) {
		return;
	}

	Composer = MakeUnique<FGCFGenericStateComposer>();

	// --- Define Player Readiness Predicates ---

	// 1. Controller: Is the owning PlayerController fully initialized and linked?
	Composer->Add(
		(uint32)EGCFPlayerReadyState::Controller,
		MakeShared<FGCFFeaturePredicate>(GFCM, PS, GCF::Names::Feature_Player_Controller)
	);

	// 2. Ability: Is the GAS (ASC) component on the PlayerState initialized?
	Composer->Add(
		(uint32)EGCFPlayerReadyState::Ability,
		MakeShared<FGCFFeaturePredicate>(GFCM, PS, GCF::Names::Feature_Player_AbilitySystem)
	);

	// 3. PlayerState: Are core PlayerState properties (UniqueId, TeamId, etc.) replicated and ready?
	Composer->Add(
		(uint32)EGCFPlayerReadyState::PlayerState,
		MakeShared<FGCFFeaturePredicate>(GFCM, PS, GCF::Names::Feature_Player_PlayerState)
	);

	// 4. Possession: Has the player successfully possessed a Pawn?
	Composer->Add(
		(uint32)EGCFPlayerReadyState::Possession,
		MakeShared<FGCFFeaturePredicate>(GFCM, PS, GCF::Names::Feature_Player_Possession)
	);

	// 5. Gameplay: Are high-level gameplay systems (HUD, Score, InputConfig) ready?
	Composer->Add(
		(uint32)EGCFPlayerReadyState::GamePlay,
		MakeShared<FGCFGameplayTagPredicate>(GFCM, PS, GCF::Names::Feature_Player_Extension, GCFGameplayTags::InitState_GameplayReady)
	);

	// Perform initial evaluation to cache the state immediately
	Reevaluate();

	// Listen for changes on the PlayerState actor
	GFCM->RegisterAndCallForActorInitState(
		PS,
		NAME_None,				// Listen to ALL features
		FGameplayTag::EmptyTag, // Trigger on ANY state change
		FActorInitStateChangedDelegate::CreateUObject(this, &ThisClass::HandleOnActorInitStateChanged),
		true
	);
}


FGCFPlayerReadyStateSnapshot UGCFPlayerReadyStateComponent::MakeSnapshot(EGCFPlayerReadyState State)
{
	return FGCFPlayerReadyStateSnapshot(GetPlayerState<APlayerState>(), State);
}