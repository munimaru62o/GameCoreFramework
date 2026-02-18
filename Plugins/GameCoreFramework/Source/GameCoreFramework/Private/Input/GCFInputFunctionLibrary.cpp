// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Input/GCFInputFunctionLibrary.h"

#include "GCFShared.h"
#include "Player/GCFPlayerController.h"
#include "Actor/GCFActorFunctionLibrary.h"
#include "Input/GCFInputConfigProvider.h"
#include "Input/GCFInputContextComponent.h"

#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"



TScriptInterface<IGCFInputConfigProvider> UGCFInputFunctionLibrary::ResolveInputConfigProvider(const UObject* Context, EGCFInputSourceType SourceType)
{
	if (!Context) {
		return nullptr;
	}

	switch (SourceType) {
		case EGCFInputSourceType::Player:
		{
			// Resolve the Controller ("Soul") and check for the interface
			const AController* Controller = UGCFActorFunctionLibrary::ResolveController(Context);
			return GCF::Context::ResolveInterface<IGCFInputConfigProvider, UGCFInputConfigProvider>(Controller);
		}
		case EGCFInputSourceType::Pawn:
		{
			// Resolve the Pawn ("Body") and check for the interface
			const APawn* Pawn = UGCFActorFunctionLibrary::ResolvePawn(Context);
			return GCF::Context::ResolveInterface<IGCFInputConfigProvider, UGCFInputConfigProvider>(Pawn);
		}
		default:
			break;
	}

	return nullptr;
}


TUniquePtr<FGCFDelegateHandle> UGCFInputFunctionLibrary::BindInputContextScoped(AController* Controller, const FOnInputContextEvaluatedNative::FDelegate& Delegate, bool bExecuteImmediately)
{
	// Find the Gatekeeper component on the controller
	UGCFInputContextComponent* ContextComp = Controller ? Controller->FindComponentByClass<UGCFInputContextComponent>() : nullptr;

	return FGCFDelegateHandle::CreateScoped(
		ContextComp,
		bExecuteImmediately,
		&UGCFInputContextComponent::RegisterAndExecuteDelegate,
		&UGCFInputContextComponent::RemoveDelegate,
		Delegate);
}


TUniquePtr<FGCFDelegateHandle> UGCFInputFunctionLibrary::BindInputComponentReadyScoped(AController* Controller, const FOnInputComponentReady::FDelegate& Delegate, bool bExecuteImmediately)
{
	// This delegate is hosted on the GCFPlayerController itself
	AGCFPlayerController* GCFController = Cast<AGCFPlayerController>(Controller);

	return FGCFDelegateHandle::CreateScoped(
		GCFController,
		bExecuteImmediately,
		&AGCFPlayerController::RegisterAndExecuteDelegate,
		&AGCFPlayerController::RemoveDelegate,
		Delegate);
}