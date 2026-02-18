// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Input/GCFInputContextComponent.h"

#include "GCFShared.h"
#include "System/Binder/GCFPossessedPawnReadyStateBinder.h"
#include "System/Binder/GCFPlayerReadyStateBinder.h"
#include "System/Binder/GCFControllerPossessionBinder.h"
#include "System/GCFDebugFunctionLibrary.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Misc/EnumClassFlags.h"


UGCFInputContextComponent::UGCFInputContextComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bWantsInitializeComponent = true;
}


FDelegateHandle UGCFInputContextComponent::RegisterAndExecuteDelegate(const FOnInputContextEvaluatedNative::FDelegate& Delegate, bool bExecuteImmediately)
{
	if (bExecuteImmediately) {
		Delegate.ExecuteIfBound(CurrentContextState, IsInputAllowed());
	}
	return OnInputContextEvaluatedNative.Add(Delegate);
}


void UGCFInputContextComponent::RemoveDelegate(const FDelegateHandle& Handle)
{
	OnInputContextEvaluatedNative.Remove(Handle);
}


void UGCFInputContextComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (AController* Controller = GetController<AController>()) {
		// Only relevant for Local Players (Human Input).
		// AI or Server-side controllers do not need input context management via this component.
		if (!Controller->IsLocalPlayerController()) {
			Deactivate();
			return;
		}

		if (UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(GetOwner())) {
			// Create binders to observe both "Soul" (Player) and "Body" (Pawn) readiness.
			// These binders automatically manage the lifetime of the delegates.
			BinderList.Emplace(FGCFPossessedPawnReadyStateBinder::CreateBinder(GFCM, Controller, FGCFOnPawnReadyStateChangedNative::FDelegate::CreateUObject(this, &ThisClass::HandlePawnReadyStateChanged)));
			BinderList.Emplace(FGCFPlayerReadyStateBinder::CreateBinder(GFCM, Controller, FGCFOnPlayerReadyStateChangedNative::FDelegate::CreateUObject(this, &ThisClass::HandlePlayerReadyStateChanged)));
		}
	}
}


void UGCFInputContextComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGCFInputContextComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	BinderList.Empty();
	Super::EndPlay(EndPlayReason);
}


void UGCFInputContextComponent::HandlePlayerReadyStateChanged(const FGCFPlayerReadyStateSnapshot& Snapshot)
{
	// Define requirements: Need both Gameplay logic and Possession to be valid.
	static const EGCFPlayerReadyState Required = EGCFPlayerReadyState::GamePlay | EGCFPlayerReadyState::Controller;
	// Check if the relevant flags have changed compared to the cached state
	if (GCF::Bitmask::HasFlagsChanged(Snapshot.State, CachedPlayerReadyState, Required)) {
		UpdateState(EGCFInputContextState::PlayerReady, GCF::Bitmask::AreFlagsSet(Snapshot.State, Required));
	}
	CachedPlayerReadyState = Snapshot.State;
}


void UGCFInputContextComponent::HandlePawnReadyStateChanged(const FGCFPawnReadyStateSnapshot& Snapshot)
{
	// Define requirements: Pawn must have Gameplay extensions and be Possessed.
	static const EGCFPawnReadyState Required = EGCFPawnReadyState::GamePlay | EGCFPawnReadyState::Possessed;
	// Check for changes
	if (GCF::Bitmask::HasFlagsChanged(Snapshot.State, CachedPawnReadyState, Required)) {
		UpdateState(EGCFInputContextState::PawnReady, GCF::Bitmask::AreFlagsSet(Snapshot.State, Required));
	}
	CachedPawnReadyState = Snapshot.State;
}


void UGCFInputContextComponent::UpdateState(EGCFInputContextState StateBit, bool bEnable)
{
	if (bEnable) {
		EnumAddFlags(CurrentContextState, StateBit);
	} else {
		EnumRemoveFlags(CurrentContextState, StateBit);
	}
	Evaluate();
}


void UGCFInputContextComponent::Evaluate()
{
	const bool bInputAllowed = IsInputAllowed();

	// Filter: Broadcast only on value change to prevent spamming.
	if (bInputAllowed == bLastEvaluatedInputEnabled) {
		return;
	}

	UE_LOG(LogGCFSystem, Log, TEXT("[%s] Input Allowed: %s -> %s"),
		   *GetClientServerContextString(this),
		   bLastEvaluatedInputEnabled ? TEXT("Enabled") : TEXT("Disabled"),
		   bInputAllowed ? TEXT("Enabled") : TEXT("Disabled"));

	UGCFDebugFunctionLibrary::SendStateMessage(this, EGCFDebugStateCategory::InputContext, bInputAllowed ? TEXT("Enabled") : TEXT("Disabled"));

	bLastEvaluatedInputEnabled = bInputAllowed;
	OnInputContextEvaluatedNative.Broadcast(CurrentContextState, bInputAllowed);
}


bool UGCFInputContextComponent::IsInputAllowed() const
{
	// Input is allowed only when BOTH Player and Pawn are fully ready.
	const EGCFInputContextState Required =
		EGCFInputContextState::PlayerReady |
		EGCFInputContextState::PawnReady;

	return EnumHasAllFlags(CurrentContextState, Required);
}
