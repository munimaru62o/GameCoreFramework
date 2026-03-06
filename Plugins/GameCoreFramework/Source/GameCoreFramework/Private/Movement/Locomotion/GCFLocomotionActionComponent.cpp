// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Movement/Locomotion/GCFLocomotionActionComponent.h"

#include "GCFShared.h"
#include "Common/GCFBitmaskUtils.h"
#include "System/Binder/GCFPawnReadyStateBinder.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Input/GCFInputConfigProvider.h"
#include "Input/GCFInputComponent.h"
#include "Movement/Locomotion/GCFLocomotionInputHandler.h"


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

	// Cache the interface to eliminate the costly Implements<U...>() search loop in the Hot Path.
	// We deliberately use TScriptInterface here to preserve Blueprint extensibility (Execute_ routing).
	// 
	// [Optimization NOTE for Production]
	// If your project requires extreme performance and you guarantee that this interface 
	// is ONLY implemented in native C++ (no Blueprint overrides), you can cast to the native 
	// pointer (IGCFLocomotionInputHandler*) and call the _Implementation functions directly.
	// This will completely bypass the VM routing overhead, but it will silently break any 
	// Blueprint overrides.
	if (APawn* Pawn = GetPawn<APawn>()) {
		// Assigning to a TScriptInterface automatically validates if the interface is implemented.
		// If the underlying object does not implement it, this will safely resolve to nullptr.
		CachedLocomotionInputHandler = Pawn;

#if !UE_BUILD_SHIPPING
		if (!CachedLocomotionInputHandler) {
			UE_LOG(LogGCFCommon, Warning, TEXT("[%s] The owning Pawn [%s] does not implement IGCFLocomotionInputHandler! Locomotion actions (Jump, Crouch) will be ignored."),
				   *GetName(), *GetNameSafe(Pawn));
		}
#endif
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

	if (GCF::Bitmask::HasFlagsChanged(CachedPawnReadyState, Snapshot.State, Required)) {
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
	if (CachedLocomotionInputHandler) {
		IGCFLocomotionInputHandler::Execute_HandleJumpInput(CachedLocomotionInputHandler.GetObject(), InputActionValue.Get<bool>());
	}
}


void UGCFLocomotionActionComponent::Input_Crouch(const FInputActionValue& InputActionValue)
{
	if (CachedLocomotionInputHandler) {
		IGCFLocomotionInputHandler::Execute_HandleCrouchInput(CachedLocomotionInputHandler.GetObject(), InputActionValue.Get<bool>());
	}
}