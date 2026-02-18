// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/Binder/GCFPlayerReadyStateBinder.h"

#include "GCFShared.h"
#include "GameFramework/PlayerState.h"
#include "System/Lifecycle/GCFStateFunctionLibrary.h"


FGCFPlayerReadyStateBinder::FGCFPlayerReadyStateBinder(UGameFrameworkComponentManager* InGFCM, AController* InController, FGCFOnPlayerReadyStateChangedNative::FDelegate&& InDelegate)
	: FGCFContextBinder(InGFCM, APlayerState::StaticClass())
	, Controller(InController)
	, Delegate(MoveTemp(InDelegate))
{};


void FGCFPlayerReadyStateBinder::Deactivate()
{
	TrackerHandle.Reset();
	FGCFContextBinder::Deactivate();
}


bool FGCFPlayerReadyStateBinder::TryResolveImmediate()
{
	// Check if the Controller already has a PlayerState assigned
	if (AController* PC = Controller.Get()) {
		if (APlayerState* PS = PC->PlayerState) {
			if (!TrackerHandle.IsValid()) {
				TrackerHandle = UGCFStateFunctionLibrary::BindPlayerReadyStateScoped(PS, Delegate);
				return true;
			}
		}
	}
	return false;
}


bool FGCFPlayerReadyStateBinder::TryResolveEvent(AActor* Actor, FName EventName)
{
	APlayerState* PS = Cast<APlayerState>(Actor);
	if (!PS || !Controller.IsValid()) {
		return false;
	}

	// Filter: Ensure we react only to the specific event designated for "Ready" or "Init"
	if (EventName == GCF::Names::Event_Player_Ready_GamePlay) {
		// Filter: Check if this PlayerState belongs to OUR controller
		if (PS->GetPlayerController() == Controller.Get()) {
			if (!TrackerHandle.IsValid()) {
				TrackerHandle = UGCFStateFunctionLibrary::BindPlayerReadyStateScoped(PS, Delegate);
				return true;
			}
		}
	}
	return false;
}