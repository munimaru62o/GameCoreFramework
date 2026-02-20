// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/Lifecycle/GCFPawnReadyStateComponent.h"

#include "GCFShared.h"
#include "System/Binder/GCFControllerPossessionBinder.h"
#include "System/Predicate/GCFFeaturePredicate.h"
#include "System/Predicate/GCFGameplayTagPredicate.h"
#include "Components/GameFrameworkComponentManager.h"
#include "System/GCFDebugFunctionLibrary.h"


UGCFPawnReadyStateComponent::UGCFPawnReadyStateComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


FDelegateHandle UGCFPawnReadyStateComponent::RegisterAndExecuteDelegate(const FGCFOnPawnReadyStateChangedNative::FDelegate& Delegate, bool bExecuteImmediately)
{
	// Ensure lazy initialization of the state composer
	EnsureBinderBuilt();

	if (bExecuteImmediately) {
		Delegate.ExecuteIfBound(MakeSnapshot(CachedState));
	}
	return OnReadyStateChangedNative.Add(Delegate);
}


void UGCFPawnReadyStateComponent::RemoveDelegate(const FDelegateHandle& Handle)
{
	OnReadyStateChangedNative.Remove(Handle);
}


void UGCFPawnReadyStateComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureBinderBuilt();
}


void UGCFPawnReadyStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Composer.Reset();
	Super::EndPlay(EndPlayReason);
}



void UGCFPawnReadyStateComponent::Reevaluate()
{
	if (!Composer) {
		return;
	}

	const EGCFPawnReadyState NewState = (EGCFPawnReadyState)Composer->Compute();
	if (NewState != CachedState) {
		CachedState = NewState;
		OnReadyStateChangedBP.Broadcast(NewState);
		OnReadyStateChangedNative.Broadcast(MakeSnapshot(NewState));

		// Send debug message,
		if (APawn* Pawn = GetPawn<APawn>()) {
			if (Pawn->IsLocallyControlled()) {
				UGCFDebugFunctionLibrary::SendPawnStateBitMessage(this, EGCFDebugStateCategory::ReadyStatePawn, NewState);
			}
		}
	}
}


void UGCFPawnReadyStateComponent::HandleOnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	Reevaluate();
}


void UGCFPawnReadyStateComponent::EnsureBinderBuilt()
{
	if (Composer) {
		return;
	}

	UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(GetOwner());
	if (!GFCM) {
		return;
	}

	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn) {
		return;
	}

	Composer = MakeUnique<FGCFGenericStateComposer>();

	// --- Configure Predicates ---
	// Define the conditions required for the Pawn to be considered "Ready".

	// 1. PawnData: Is the core data asset loaded and applied?
	Composer->Add(
		(uint32)EGCFPawnReadyState::PawnData,
		MakeShared<FGCFFeaturePredicate>(GFCM, Pawn, GCF::Names::Feature_Pawn_PawnData)
	);

	// 2. Ability: Is the Gameplay Ability System (ASC) initialized?
	Composer->Add(
		(uint32)EGCFPawnReadyState::Ability,
		MakeShared<FGCFFeaturePredicate>(GFCM, Pawn, GCF::Names::Feature_Pawn_Ability)
	);

	// 3. Possessed: Is a valid Controller possessing this Pawn?
	Composer->Add(
		(uint32)EGCFPawnReadyState::Possessed,
		MakeShared<FGCFFeaturePredicate>(GFCM, Pawn, GCF::Names::Feature_Pawn_Possessed)
	);

	// 4. Gameplay: Are additional extensions (Input, UI, etc.) ready?
	Composer->Add(
		(uint32)EGCFPawnReadyState::GamePlay,
		MakeShared<FGCFGameplayTagPredicate>(GFCM, Pawn, GCF::Names::Feature_Pawn_Extension, GCFGameplayTags::InitState_GameplayReady)
	);

	// Perform an initial evaluation to cache the current state immediately after construction.
	Reevaluate();

	// Register to listen for ANY init state change on this actor.
	GFCM->RegisterAndCallForActorInitState(
		Pawn,
		NAME_None,				// Listen to ALL registered features
		FGameplayTag::EmptyTag, // Trigger on ANY state change
		FActorInitStateChangedDelegate::CreateUObject(this, &ThisClass::HandleOnActorInitStateChanged),
		true
	);
}


FGCFPawnReadyStateSnapshot UGCFPawnReadyStateComponent::MakeSnapshot(EGCFPawnReadyState State)
{
	return FGCFPawnReadyStateSnapshot(GetPawn<APawn>(), State);
}