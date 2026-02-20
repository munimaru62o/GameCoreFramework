// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Interaction/GCFInteractionComponent.h"

#include "GCFShared.h"
#include "Interaction/Mode/GCFInteractionMode.h"
#include "Interaction/GCFInteractable.h"
#include "AbilitySystem/GCFAbilitySystemFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "Actor/Data/GCFPawnData.h"
#include "Input/GCFInputConfigProvider.h"
#include "Input/GCFInputComponent.h"


UGCFInteractionComponent::UGCFInteractionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true; 
	PrimaryComponentTick.TickInterval = 0.1f; // Optimization: Run 10 times per second is usually enough.

	InteractTraceChannel = GCF_TRACE_CHANNEL_INTERACTION;
	DefaultCameraModeTag = GCFGameplayTags::Camera_Mode_ThirdPerson;
}

void UGCFInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only run detection on the Local Player Controller.
	if (AController* Controller = GetController<AController>()) {
		if (!Controller->IsLocalController()) {
			Deactivate();
			SetComponentTickEnabled(false);
			return;
		}
	}

	// Subscribe to Camera Mode changes
	if (UWorld* World = GetWorld()) {
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(World);
		CameraMessageHandle = MakeUnique<FGCFMessageSubscription>(
			World,
			MessageSubsystem.RegisterListener<FGCFCameraModeChangedMessage>(
			GCFGameplayTags::Message_Camera_ModeChange,
			this,
			&ThisClass::OnCameraModeMessageReceived));
	}

	UpdateActiveCameraMode(DefaultCameraModeTag);
}


void UGCFInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CameraMessageHandle.Reset();
	Super::EndPlay(EndPlayReason);
}


void UGCFInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CurrentActiveSetting || !ActiveModeInstance) {
		return;
	}

	APlayerController* PC = GetController<APlayerController>();
	if (!PC || !PC->IsLocalController()) {
		return;
	}

	FGCFInteractionSearchParams Params;
	Params.SourceComponent = this;
	Params.PC = PC;
	Params.Pawn = GetPawn<APawn>();
	Params.Distance = CurrentActiveSetting->TraceDistance;
	Params.Channel = InteractTraceChannel;
	Params.bShowDebug = bShowDebugTrace;

	// 1. Find Target via Strategy
	AActor* NewHitActor = ActiveModeInstance->FindTarget(Params);

	// 2. Validate Interface Implementation
	if (NewHitActor && !NewHitActor->Implements<UGCFInteractable>()) {
		NewHitActor = nullptr;
	}

	// 3. Update Focus State
	if (NewHitActor != FocusedTarget) {
		UpdateFocusedTarget(NewHitActor);
	}
}


void UGCFInteractionComponent::UpdateFocusedTarget(AActor* NewTarget)
{
	if (FocusedTarget) {
		IGCFInteractable::Execute_OnEndFocus(FocusedTarget);
	}

	FocusedTarget = NewTarget;

	if (FocusedTarget) {
		IGCFInteractable::Execute_OnBeginFocus(FocusedTarget);
	}
}


void UGCFInteractionComponent::UpdateActiveCameraMode(FGameplayTag NewCameraModeTag)
{
	if (!InteractionConfig || ActiveCameraModeTag.MatchesTagExact(NewCameraModeTag)) {
		return;
	}

	ActiveCameraModeTag = NewCameraModeTag;
	CurrentActiveSetting = InteractionConfig->FindSettingForCameraMode(ActiveCameraModeTag);

	if (CurrentActiveSetting && CurrentActiveSetting->InteractionModeClass) {
		TSubclassOf<UGCFInteractionMode> TargetClass = CurrentActiveSetting->InteractionModeClass;

		// Get from cache or create new
		TObjectPtr<UGCFInteractionMode>& FoundInst = ModeInstanceCache.FindOrAdd(TargetClass);
		if (!FoundInst) {
			FoundInst = NewObject<UGCFInteractionMode>(this, TargetClass);
		}
		ActiveModeInstance = FoundInst;
	} else {
		ActiveModeInstance = nullptr;
	}

	// Reset focus when switching modes to prevent stale targets
	UpdateFocusedTarget(nullptr);
}


void UGCFInteractionComponent::OnCameraModeMessageReceived(FGameplayTag Channel, const FGCFCameraModeChangedMessage& Message)
{
	if (AController* Controller = GetController<AController>()) {
		if (Message.Controller == Controller) {
			UpdateActiveCameraMode(Message.NewCameraModeTag);
		}
	}
}