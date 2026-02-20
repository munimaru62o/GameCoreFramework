// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Input/GCFPawnInputBridgeComponent.h"

#include "Input/GCFInputComponent.h"
#include "Input/GCFInputConfig.h"
#include "Input/GCFInputBindingManagerComponent.h"
#include "Input/GCFAbilityInputRouterComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "System/Lifecycle/GCFPawnReadyStateComponent.h"
#include "System/Binder/GCFPawnReadyStateBinder.h"
#include "Components/GameFrameworkComponentManager.h"


UGCFPawnInputBridgeComponent::UGCFPawnInputBridgeComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bWantsInitializeComponent = true;
}


void UGCFPawnInputBridgeComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Setup the binder to wait for this Pawn to become "Ready".
	// The actual input registration happens only after the Pawn is fully initialized and possessed.
	if (UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(GetOwner())) {
		PawnReadyBinder = FGCFPawnReadyStateBinder::CreateBinder(GFCM, GetPawn<APawn>(), FGCFOnPawnReadyStateChangedNative::FDelegate::CreateUObject(this, &ThisClass::HandlePawnReadyStateChanged));
	}
}


void UGCFPawnInputBridgeComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UGCFPawnInputBridgeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PawnReadyBinder.Reset();
	Super::EndPlay(EndPlayReason);
}


void UGCFPawnInputBridgeComponent::AddInputConfig(const UGCFInputConfig* InputConfig)
{
	InputConfigList.AddUnique(InputConfig);

	// Notify the Binding Manager on the Controller that new configs are available.
	// This triggers a re-evaluation of bindings (e.g., equipping a weapon adds new actions).
	if (AController* Controller = GetController<AController>()) {
		if (UGCFInputBindingManagerComponent* Manager = Controller->FindComponentByClass<UGCFInputBindingManagerComponent>()) {
			Manager->NotifyBindingContextChanged();
		}
	}
}


void UGCFPawnInputBridgeComponent::RemoveInputConfig(const UGCFInputConfig* InputConfig)
{
	InputConfigList.Remove(InputConfig);

	// TODO: implement
}


TArray<FGCFBindingReceipt> UGCFPawnInputBridgeComponent::HandleInputBinding(UGCFInputComponent* InputComponent, TScriptInterface<IGCFInputConfigProvider> Provider)
{
	TArray<FGCFBindingReceipt> Receipts{};

	if (!Provider) {
		return Receipts;
	}

	for (const UGCFInputConfig* Config : Provider->GetInputConfigList()) {
		if (!Config) {
			continue;
		}
		FGCFInputBinder Binder(InputComponent, Config, Receipts);
		Binder.BindAbility(this, &ThisClass::HandleInputPressed, &ThisClass::HandleInputReleased);
	}

	return Receipts;
}


void UGCFPawnInputBridgeComponent::HandlePawnReadyStateChanged(const FGCFPawnReadyStateSnapshot& Snapshot)
{
	// Requirements: "Possessed" (Controller assigned) AND "GamePlay" (Extensions loaded).
	static const EGCFPawnReadyState Required = EGCFPawnReadyState::Possessed | EGCFPawnReadyState::GamePlay;
	if (GCF::Bitmask::HasFlagsChanged(Snapshot.State, CachedPawnReadyState, Required)) {
		if (GCF::Bitmask::AreFlagsSet(Snapshot.State, Required)) {
			// 1. Resolve dependencies
			if (AController* Controller = GetController<AController>()) {
				AbilityInputRouter = Controller->FindComponentByClass<UGCFAbilityInputRouterComponent>();
			}
			// 2. Register this component as an Input Source to the Manager.
			// This tells the Controller: "I am your Body, and here are my controls."
			GCF_REGISTER_INPUT_BINDING(this, &ThisClass::HandleInputBinding);
		}
	}
	CachedPawnReadyState = Snapshot.State;
}


void UGCFPawnInputBridgeComponent::HandleInputPressed(FGameplayTag InputTag)
{
	// Forward the event to the centralized router on the Controller (Soul).
	if (AbilityInputRouter.Get()) {
		AbilityInputRouter->RouteInputTag(InputTag, true);
	}
}


void UGCFPawnInputBridgeComponent::HandleInputReleased(FGameplayTag InputTag)
{
	if (AbilityInputRouter.Get()) {
		AbilityInputRouter->RouteInputTag(InputTag, false);
	}
}
