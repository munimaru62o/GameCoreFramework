// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Actor/Vehicle/GCFVehicleControlComponent.h"

#include "GCFShared.h"
#include "System/Binder/GCFPawnReadyStateBinder.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Input/GCFInputConfigProvider.h"
#include "Input/GCFInputComponent.h"
#include "Actor/Vehicle/GCFWheeledVehiclePawn.h"


UGCFVehicleControlComponent::UGCFVehicleControlComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	CachedPawnReadyState = EGCFPawnReadyState::None;
}


void UGCFVehicleControlComponent::BeginPlay()
{
	Super::BeginPlay();

	// Observe the Pawn's state. We don't bind inputs immediately here;
	// instead, we wait for HandlePawnReadyStateChanged to confirm the Pawn is ready.
	if (UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(GetOwner())) {
		Binder = FGCFPawnReadyStateBinder::CreateBinder(GFCM, GetPawn<APawn>(), FGCFOnPawnReadyStateChangedNative::FDelegate::CreateUObject(this, &ThisClass::HandlePawnReadyStateChanged));
	}
}


void UGCFVehicleControlComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Binder.Reset();
	Super::EndPlay(EndPlayReason);
}


void UGCFVehicleControlComponent::HandlePawnReadyStateChanged(const FGCFPawnReadyStateSnapshot& Snapshot)
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


TArray<FGCFBindingReceipt> UGCFVehicleControlComponent::HandleInputBinding(UGCFInputComponent* InputComponent, TScriptInterface<IGCFInputConfigProvider> Provider)
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
		InputBinder.Bind(GCFGameplayTags::InputTag_Vehicle_HandBrake, ETriggerEvent::Triggered, this, &ThisClass::Input_HandBrake);
		InputBinder.Bind(GCFGameplayTags::InputTag_Vehicle_Light, ETriggerEvent::Triggered, this, &ThisClass::Input_Light);
	}
	return Receipts;
}


void UGCFVehicleControlComponent::Input_HandBrake(const FInputActionValue& InputActionValue)
{
	if (AGCFWheeledVehiclePawn* Vehicle = GetPawn<AGCFWheeledVehiclePawn>()) {
		Vehicle->ToggleHandBrakeInput();
	}
}


void UGCFVehicleControlComponent::Input_Light(const FInputActionValue& InputActionValue)
{
	if (AGCFWheeledVehiclePawn* Vehicle = GetPawn<AGCFWheeledVehiclePawn>()) {
		Vehicle->ToggleHeadLightInput();
	}
}
