// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/Lifecycle/GCFStateFunctionLibrary.h"

#include "GCFShared.h"
#include "Common/GCFTypes.h"
#include "System/Lifecycle/GCFPawnReadyStateComponent.h"
#include "System/Lifecycle/GCFPlayerReadyStateComponent.h"
#include "Player/GCFControllerPossessionComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Controller.h"


TUniquePtr<FGCFDelegateHandle> UGCFStateFunctionLibrary::BindPawnReadyStateScoped(APawn* Pawn, const FGCFOnPawnReadyStateChangedNative::FDelegate& Delegate, bool bExecuteImmediately)
{
	return FGCFDelegateHandle::CreateScoped(
		UGCFPawnReadyStateComponent::FindGCFPawnReadyStateComponent(Pawn),
		bExecuteImmediately,
		&UGCFPawnReadyStateComponent::RegisterAndExecuteDelegate,
		&UGCFPawnReadyStateComponent::RemoveDelegate,
		Delegate);
}


TUniquePtr<FGCFDelegateHandle> UGCFStateFunctionLibrary::BindPlayerReadyStateScoped(APlayerState* PlayerState, const FGCFOnPlayerReadyStateChangedNative::FDelegate& Delegate, bool bExecuteImmediately)
{
	return FGCFDelegateHandle::CreateScoped(
		UGCFPlayerReadyStateComponent::FindGCFPlayerReadyStateComponent(PlayerState),
		bExecuteImmediately,
		&UGCFPlayerReadyStateComponent::RegisterAndExecuteDelegate,
		&UGCFPlayerReadyStateComponent::RemoveDelegate,
		Delegate);
}


TUniquePtr<FGCFDelegateHandle> UGCFStateFunctionLibrary::BindPossessionScoped(AController* Controller, const FOnPossessedPawnChangedNative::FDelegate& Delegate, bool bExecuteImmediately)
{
	return FGCFDelegateHandle::CreateScoped(
		UGCFControllerPossessionComponent::FindGCFControllerPossessionComponent(Controller),
		bExecuteImmediately,
		&UGCFControllerPossessionComponent::RegisterAndExecuteDelegate,
		&UGCFControllerPossessionComponent::RemoveDelegate,
		Delegate);
}