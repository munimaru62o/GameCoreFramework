// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Actor/Avatar/GCFAvatarControlComponent.h"

#include "GCFShared.h"
#include "System/Binder/GCFPawnReadyStateBinder.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Actor/Avatar/GCFAvatarPawn.h"
#include "Input/GCFInputConfigProvider.h"
#include "Input/GCFInputComponent.h"


UGCFAvatarControlComponent::UGCFAvatarControlComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	CachedPawnReadyState = EGCFPawnReadyState::None;
}


void UGCFAvatarControlComponent::BeginPlay()
{
	Super::BeginPlay();

	// Observe the Pawn's state. We don't bind inputs immediately here;
	// instead, we wait for HandlePawnReadyStateChanged to confirm the Pawn is ready.
	if (UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(GetOwner())) {
		Binder = FGCFPawnReadyStateBinder::CreateBinder(GFCM, GetPawn<APawn>(), FGCFOnPawnReadyStateChangedNative::FDelegate::CreateUObject(this, &ThisClass::HandlePawnReadyStateChanged));
	}
}


void UGCFAvatarControlComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Binder.Reset();
	Super::EndPlay(EndPlayReason);
}


void UGCFAvatarControlComponent::HandlePawnReadyStateChanged(const FGCFPawnReadyStateSnapshot& Snapshot)
{
	// We require both "Possessed" (Input Routing established) and "GamePlay" (Logic Initialized).
	static const EGCFPawnReadyState Required = EGCFPawnReadyState::Possessed | EGCFPawnReadyState::GamePlay;

	if (GCF::Bitmask::HasFlagsChanged(Snapshot.State, CachedPawnReadyState, Required)) {
		if (GCF::Bitmask::AreFlagsSet(Snapshot.State, Required)) {
			// Now that the pawn is fully ready, register the input bindings.
			GCF_REGISTER_INPUT_BINDING(this, &ThisClass::HandleInputBinding);
		}
	}
	CachedPawnReadyState = Snapshot.State;
}


TArray<FGCFBindingReceipt> UGCFAvatarControlComponent::HandleInputBinding(UGCFInputComponent* InputComponent, TScriptInterface<IGCFInputConfigProvider> Provider)
{
	TArray<FGCFBindingReceipt> Receipts{};
	if (!Provider) {
		return Receipts;
	}

	for (const UGCFInputConfig* Config : Provider->GetInputConfigList()) {
		if (!Config) {
			continue;
		}
		FGCFInputBinder InputBinder(InputComponent, Config, Receipts);
		InputBinder.Bind(GCFGameplayTags::InputTag_Character_Jump, ETriggerEvent::Triggered, this, &ThisClass::Input_Jump);
		InputBinder.Bind(GCFGameplayTags::InputTag_Character_Jump, ETriggerEvent::Completed, this, &ThisClass::Input_Jump);
	}
	return Receipts;
}


void UGCFAvatarControlComponent::Input_Jump(const FInputActionValue& InputActionValue)
{
	if (AGCFAvatarPawn* AvatarPawn = GetPawn<AGCFAvatarPawn>()) {
		const bool bIsPressed = InputActionValue.Get<bool>();
		AvatarPawn->HandleJumpInput(bIsPressed);
	}
}