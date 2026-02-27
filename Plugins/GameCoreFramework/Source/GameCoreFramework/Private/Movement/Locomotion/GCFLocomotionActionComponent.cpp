// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Movement/Locomotion/GCFLocomotionActionComponent.h"

#include "GCFShared.h"
#include "System/Binder/GCFPawnReadyStateBinder.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Input/GCFInputConfigProvider.h"
#include "Input/GCFInputComponent.h"
#include "Movement/Locomotion/GCFLocomotionInputHandler.h"
#include "Movement/GCFMovementFunctionLibrary.h"


UGCFLocomotionActionComponent::UGCFLocomotionActionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	CachedPawnReadyState = EGCFPawnReadyState::None;
}


void UGCFLocomotionActionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Observe the Pawn's state. We don't bind inputs immediately here;
	// instead, we wait for HandlePawnReadyStateChanged to confirm the Pawn is ready.
	if (UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(GetOwner())) {
		Binder = FGCFPawnReadyStateBinder::CreateBinder(GFCM, GetPawn<APawn>(), FGCFOnPawnReadyStateChangedNative::FDelegate::CreateUObject(this, &ThisClass::HandlePawnReadyStateChanged));
	}
}


void UGCFLocomotionActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Binder.Reset();
	Super::EndPlay(EndPlayReason);
}


void UGCFLocomotionActionComponent::HandlePawnReadyStateChanged(const FGCFPawnReadyStateSnapshot& Snapshot)
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


TArray<FGCFBindingReceipt> UGCFLocomotionActionComponent::HandleInputBinding(UGCFInputComponent* InputComponent, TScriptInterface<IGCFInputConfigProvider> Provider)
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

		// Note: Adjusted the Tags to be more generic since this is no longer "Character" specific.
		InputBinder.Bind(GCFGameplayTags::InputTag_Jump, ETriggerEvent::Triggered, this, &ThisClass::Input_Jump);
		InputBinder.Bind(GCFGameplayTags::InputTag_Jump, ETriggerEvent::Completed, this, &ThisClass::Input_Jump);

		InputBinder.Bind(GCFGameplayTags::InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch);
		InputBinder.Bind(GCFGameplayTags::InputTag_Crouch, ETriggerEvent::Completed, this, &ThisClass::Input_Crouch);
	}
	return Receipts;
}


void UGCFLocomotionActionComponent::Input_Jump(const FInputActionValue& InputActionValue)
{
	if (APawn* Pawn = GetPawn<APawn>()) {
		if (TScriptInterface<IGCFLocomotionInputHandler> Handler = UGCFMovementFunctionLibrary::ResolveLocomotionInputHandler(Pawn)) {
			IGCFLocomotionInputHandler::Execute_HandleJumpInput(Handler.GetObject(), InputActionValue.Get<bool>());
		}
	}
}

void UGCFLocomotionActionComponent::Input_Crouch(const FInputActionValue& InputActionValue)
{
	if (APawn* Pawn = GetPawn<APawn>()) {
		if (TScriptInterface<IGCFLocomotionInputHandler> Handler = UGCFMovementFunctionLibrary::ResolveLocomotionInputHandler(Pawn)) {
			IGCFLocomotionInputHandler::Execute_HandleCrouchInput(Handler.GetObject(), InputActionValue.Get<bool>());
		}
	}
}