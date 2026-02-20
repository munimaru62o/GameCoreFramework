// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/Lifecycle/GCFPossessionContextComponent.h"

#include "GCFShared.h"
#include "Actor/GCFActorFunctionLibrary.h"
#include "Components/GameFrameworkComponentManager.h"
#include "System/Lifecycle/GCFStateFunctionLibrary.h"
#include "System/Lifecycle/GCFGameFeatureFunctionLibrary.h"
#include "System/GCFDebugFunctionLibrary.h"
#include "GameFramework/PlayerState.h"


UGCFPossessionContextComponent::UGCFPossessionContextComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


void UGCFPossessionContextComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AController* Controller = GetController<AController>()) {
		PossessionTrackerHandle = UGCFStateFunctionLibrary::BindPossessionScoped(
			Controller, FOnPossessedPawnChangedNative::FDelegate::CreateUObject(this, &ThisClass::HandlePawnChanged)
		);
	}
}


void UGCFPossessionContextComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PossessionTrackerHandle.Reset();
	Super::EndPlay(EndPlayReason);
}


void UGCFPossessionContextComponent::HandlePawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	// [Order matters] Always notify "Unpossess" first.
	// This ensures that systems monitoring the old Pawn clean up before the new Pawn starts logic.
	if (OldPawn) {
		UGCFGameFeatureFunctionLibrary::InitFeature(this, OldPawn, GCF::Names::Feature_Pawn_Possessed);
		BroadcastEvent(OldPawn, GCF::Names::Event_Pawn_OnUnPossessed);
	}

	// Notify "Possess" for the new Pawn.
	if (NewPawn) {
		UGCFGameFeatureFunctionLibrary::ReadyFeature(this, NewPawn, GCF::Names::Feature_Pawn_Possessed);
		BroadcastEvent(NewPawn, GCF::Names::Event_Pawn_OnPossessed);
	}

	CheckAndUpdatePlayerPossessionState();

	if (IsLocalController()) {
		UGCFDebugFunctionLibrary::SendLogMessage(this, EGCFDebugLogVerbosity::Info, FString::Printf(TEXT("PawnChanged: %s to %s"), *GetNameSafe(OldPawn), *GetNameSafe(NewPawn)));
		UGCFDebugFunctionLibrary::SendStateMessage(this, EGCFDebugStateCategory::Possession, *GetNameSafe(NewPawn));
	}
}


void UGCFPossessionContextComponent::CheckAndUpdatePlayerPossessionState()
{
	if (APlayerState* PlayerState = UGCFActorFunctionLibrary::ResolvePlayerState(this)) {
		if (AController* Controller = GetController<AController>()) {
			if (APawn* CurrentPawn = Controller->GetPawn()) {
				UGCFGameFeatureFunctionLibrary::ReadyFeature(this, PlayerState, GCF::Names::Feature_Player_Possession);
			} else {
				UGCFGameFeatureFunctionLibrary::InitFeature(this, PlayerState, GCF::Names::Feature_Player_Possession);
			}
		}
	}
}


void UGCFPossessionContextComponent::BroadcastEvent(APawn* Pawn, FName EventName)
{
	if (!Pawn) {
		return;
	}

	if (UGameFrameworkComponentManager* Manager = UGameFrameworkComponentManager::GetForActor(Pawn)) {
		Manager->SendGameFrameworkComponentExtensionEvent(Pawn, EventName);
	}
}