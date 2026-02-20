// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Movement/GCFMovementControlComponent.h"

#include "GCFShared.h"
#include "Actor/Data/GCFPawnData.h"
#include "Actor/Data/GCFPawnDataProvider.h"
#include "Camera/GCFCameraFunctionLibrary.h"
#include "Movement/GCFLocomotionHandler.h"
#include "Movement/GCFMovementFunctionLibrary.h"
#include "Input/GCFInputConfigProvider.h"
#include "Input/GCFInputConfig.h"
#include "Input/GCFInputComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "System/Binder/GCFPossessedPawnReadyStateBinder.h"

#include "Misc/EnumClassFlags.h"


UGCFMovementControlComponent::UGCFMovementControlComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	CachedPolicy = EGCFMovementRotationPolicy::CameraDriven;
}


void UGCFMovementControlComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AController* Controller = GetController<AController>()) {
		// Only relevant for Local Players (Human Input).
		// AI or Server-side controllers do not need input context management via this component.
		if (!Controller->IsLocalPlayerController()) {
			Deactivate();
			return;
		}

		// Listen for rotation policy changes (from Camera System)
		Handle = UGCFCameraFunctionLibrary::BindRotationPolicyScoped(
			Controller,
			FGCFOnMovementRotationPolicyChanged::FDelegate::CreateUObject(this, &ThisClass::HandleMovementRotationPolicyChanged)
		);

		// Auto-register input bindings
		GCF_REGISTER_INPUT_BINDING(this, &ThisClass::HandleInputBinding);
	}
}

void UGCFMovementControlComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Handle.Reset();
	Super::EndPlay(EndPlayReason);
}


void UGCFMovementControlComponent::HandleMovementRotationPolicyChanged(EGCFMovementRotationPolicy Policy)
{
	CachedPolicy = Policy;
}


TArray<FGCFBindingReceipt> UGCFMovementControlComponent::HandleInputBinding(UGCFInputComponent* InputComponent, TScriptInterface<IGCFInputConfigProvider> Provider)
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
		InputBinder.Bind(GCFGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
		InputBinder.Bind(GCFGameplayTags::InputTag_Move, ETriggerEvent::Completed, this, &ThisClass::Input_Move_Completed);

		InputBinder.Bind(GCFGameplayTags::InputTag_MoveUp, ETriggerEvent::Triggered, this, &ThisClass::Input_MoveUp);
		InputBinder.Bind(GCFGameplayTags::InputTag_MoveUp, ETriggerEvent::Completed, this, &ThisClass::Input_MoveUp_Completed);
	}

	return Receipts;
}


void UGCFMovementControlComponent::Input_Move(const FInputActionValue& Value)
{
	if (AController* Controller = GetController<AController>()) {

		// Determine the rotation basis for movement (Camera, World, or Pawn relative)
		const FRotator MovementRotation = CalcMovementRotation(Controller);
		const FVector2D MovementVector = Value.Get<FVector2D>();

		if (APawn* Pawn = GetPawn<APawn>()) {
			if (TScriptInterface<IGCFLocomotionHandler> Handler = UGCFMovementFunctionLibrary::ResolveLocomotionHandler(Pawn)) {
				IGCFLocomotionHandler::Execute_HandleMoveInput(Handler.GetObject(), MovementVector, MovementRotation);
			}
		}
	}
}

/**
 * Triggered when the directional movement input is released (ETriggerEvent::Completed).
 * Explicitly notifies the locomotion handler to halt horizontal movement by passing a zero vector.
 * This prevents the pawn from continuously moving if the input stream abruptly stops.
 */
void UGCFMovementControlComponent::Input_Move_Completed(const FInputActionValue& Value)
{
	if (AController* Controller = GetController<AController>()) {
		if (APawn* Pawn = GetPawn<APawn>()) {
			if (TScriptInterface<IGCFLocomotionHandler> Handler = UGCFMovementFunctionLibrary::ResolveLocomotionHandler(Pawn)) {
				IGCFLocomotionHandler::Execute_HandleMoveInput(Handler.GetObject(), FVector2D::ZeroVector, Controller->GetControlRotation());
			}
		}
	}
}


void UGCFMovementControlComponent::Input_MoveUp(const FInputActionValue& Value)
{
	const float UpValue = Value.Get<float>();

	if (APawn* Pawn = GetPawn<APawn>()) {
		if (TScriptInterface<IGCFLocomotionHandler> Handler = UGCFMovementFunctionLibrary::ResolveLocomotionHandler(Pawn)) {
			IGCFLocomotionHandler::Execute_HandleMoveUpInput(Handler.GetObject(), UpValue);
		}
	}
}


/**
 * Triggered when the vertical movement input is released (ETriggerEvent::Completed).
 * Explicitly notifies the locomotion handler to halt vertical movement (e.g., flying, swimming) by passing 0.0f.
 */
void UGCFMovementControlComponent::Input_MoveUp_Completed(const FInputActionValue& Value)
{
	if (APawn* Pawn = GetPawn<APawn>()) {
		if (TScriptInterface<IGCFLocomotionHandler> Handler = UGCFMovementFunctionLibrary::ResolveLocomotionHandler(Pawn)) {
			IGCFLocomotionHandler::Execute_HandleMoveUpInput(Handler.GetObject(), 0.0f);
		}
	}
}


FRotator UGCFMovementControlComponent::CalcMovementRotation(AController* Controller) const
{
	if (!Controller) {
		return FRotator::ZeroRotator;
	}

	switch (CachedPolicy) {
		case EGCFMovementRotationPolicy::Invalid:
		case EGCFMovementRotationPolicy::CameraDriven:
		default:
			// Standard TPS: Move relative to Camera Yaw
			return FRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		case EGCFMovementRotationPolicy::FreeLook:
			// FreeLook: Often implies movement relative to the Pawn itself, or World?
			// Usually, FreeLook implies the camera rotates independently, but movement is still camera-relative 
			// OR movement is relative to the character's current facing.
			// Here we assume standard Camera-Relative for typical modern games.
			return FRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		case EGCFMovementRotationPolicy::Fixed:
			// Fixed Camera (e.g., TopDown, SideScroll):
			// Movement inputs are absolute World Directions (Up=North/Forward, Right=East/Right).
			return FRotator::ZeroRotator;
	}
}