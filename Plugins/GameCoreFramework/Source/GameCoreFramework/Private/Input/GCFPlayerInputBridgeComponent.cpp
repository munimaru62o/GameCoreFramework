// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Input/GCFPlayerInputBridgeComponent.h"

#include "Input/GCFAbilityInputRouterComponent.h"
#include "Input/GCFInputComponent.h"
#include "Input/GCFInputBindingManagerComponent.h"
#include "GameFramework/PlayerController.h"


UGCFPlayerInputBridgeComponent::UGCFPlayerInputBridgeComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bWantsInitializeComponent = true;
}


void UGCFPlayerInputBridgeComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (AController* Controller = GetController<AController>()) {
		// Only relevant for Local Players (Input Source).
		if (!Controller->IsLocalController()) {
			Deactivate();
			return;
		}
	}
}


void UGCFPlayerInputBridgeComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache the router reference.
	if (AController* Controller = GetController<AController>()) {
		AbilityInputRouter = Controller->FindComponentByClass<UGCFAbilityInputRouterComponent>();
	}

	// Register this component immediately as a persistent input source.
	// Unlike the Pawn bridge, we don't need to wait for "Possession" or "ReadyState"
	// because the Controller exists as long as the player is connected.
	GCF_REGISTER_INPUT_BINDING(this, &ThisClass::HandleInputBinding);
}


void UGCFPlayerInputBridgeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}


void UGCFPlayerInputBridgeComponent::AddInputConfig(const UGCFInputConfig* InputConfig)
{
	InputConfigList.AddUnique(InputConfig);

	// Notify the Binding Manager to refresh bindings dynamically.
	if (AController* Controller = GetOwner<AController>()) {
		if (UGCFInputBindingManagerComponent* Manager = Controller->FindComponentByClass<UGCFInputBindingManagerComponent>()) {
			Manager->NotifyBindingContextChanged();
		}
	}
}


void UGCFPlayerInputBridgeComponent::RemoveInputConfig(const UGCFInputConfig* InputConfig)
{
	InputConfigList.Remove(InputConfig);

	// TODO: implement unbinding logic if necessary
}


TArray<FGCFBindingReceipt> UGCFPlayerInputBridgeComponent::HandleInputBinding(UGCFInputComponent* InputComponent, TScriptInterface<IGCFInputConfigProvider> Provider)
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


void UGCFPlayerInputBridgeComponent::HandleInputPressed(const FGameplayTag InputTag)
{
	if (AbilityInputRouter.Get()) {
		AbilityInputRouter->RouteInputTag(InputTag, /*bPressed=*/true);
	}
}


void UGCFPlayerInputBridgeComponent::HandleInputReleased(const FGameplayTag InputTag)
{
	if (AbilityInputRouter.Get()) {
		AbilityInputRouter->RouteInputTag(InputTag, /*bPressed=*/false);
	}
}