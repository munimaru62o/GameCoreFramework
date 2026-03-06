// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Movement/Locomotion/GCFLocomotionDirectionComponent.h"

#include "GCFShared.h"
#include "Camera/GCFCameraFunctionLibrary.h"
#include "Movement/Locomotion/GCFLocomotionInputHandler.h"
#include "Input/GCFInputConfigProvider.h"
#include "Input/GCFInputConfig.h"
#include "Input/GCFInputComponent.h"
#include "System/Binder/GCFControllerPossessionBinder.h"
#include "Components/GameFrameworkComponentManager.h"


UGCFLocomotionDirectionComponent::UGCFLocomotionDirectionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	CachedPolicy = EGCFMovementRotationPolicy::CameraDriven;
}


void UGCFLocomotionDirectionComponent::BeginPlay()
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

		if (UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(Controller)) {
			PossessionBinder = FGCFControllerPossessionBinder::CreateBinder(
				GFCM, Controller, FGCFBooleanStateSignature::CreateUObject(this, &ThisClass::HandlePossessedPawnChanged));
		}

		// Auto-register input bindings
		GCF_REGISTER_INPUT_BINDING(this, &ThisClass::HandleInputBinding);
	}
}

void UGCFLocomotionDirectionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Handle.Reset();
	PossessionBinder.Reset();
	Super::EndPlay(EndPlayReason);
}


void UGCFLocomotionDirectionComponent::HandleMovementRotationPolicyChanged(EGCFMovementRotationPolicy Policy)
{
	CachedPolicy = Policy;
}


void UGCFLocomotionDirectionComponent::HandlePossessedPawnChanged(AActor* Actor, bool bPossessed)
{
	if (bPossessed) {
		APawn* PossessedPawn = Cast<APawn>(Actor);

		// Cache the interface to eliminate the costly Implements<U...>() search loop in the Hot Path.
		// We deliberately use TScriptInterface here to preserve Blueprint extensibility (Execute_ routing).
		// 
		// [Optimization NOTE for Production]
		// If your project requires extreme performance and you guarantee that this interface 
		// is ONLY implemented in native C++ (no Blueprint overrides), you can cast to the native 
		// pointer (IGCFLocomotionInputHandler*) and call the _Implementation functions directly.
		// This will completely bypass the VM routing overhead, but it will silently break any 
		// Blueprint overrides.
		
		// Assigning to a TScriptInterface automatically validates if the interface is implemented.
		// If the underlying object does not implement it, this will safely resolve to nullptr.
		CachedLocomotionInputHandler = PossessedPawn;

#if !UE_BUILD_SHIPPING
		if (!CachedLocomotionInputHandler) {
			UE_LOG(LogGCFCommon , Warning, TEXT("[%s] The possessed Pawn [%s] does not implement IGCFLocomotionInputHandler! Directional input (Move) will be ignored."),
				   *GetName(), *GetNameSafe(PossessedPawn));
		}
#endif
	} else {
		// Clear cache on unpossess
		CachedLocomotionInputHandler = nullptr;
	}
}


TArray<FGCFBindingReceipt> UGCFLocomotionDirectionComponent::HandleInputBinding(UGCFInputComponent* InputComponent, TScriptInterface<IGCFInputConfigProvider> Provider)
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


void UGCFLocomotionDirectionComponent::Input_Move(const FInputActionValue& Value)
{
	if (AController* Controller = GetController<AController>()) {

		// Determine the rotation basis for movement (Camera, World, or Pawn relative)
		const FRotator MovementRotation = CalcMovementRotation(Controller);
		const FVector2D MovementVector = Value.Get<FVector2D>();

		if (CachedLocomotionInputHandler) {
			IGCFLocomotionInputHandler::Execute_HandleMoveInput(CachedLocomotionInputHandler.GetObject(), MovementVector, MovementRotation);
		}
	}
}

/**
 * Triggered when the directional movement input is released (ETriggerEvent::Completed).
 * Explicitly notifies the locomotion handler to halt horizontal movement by passing a zero vector.
 * This prevents the pawn from continuously moving if the input stream abruptly stops.
 */
void UGCFLocomotionDirectionComponent::Input_Move_Completed(const FInputActionValue& Value)
{
	if (AController* Controller = GetController<AController>()) {
		if (CachedLocomotionInputHandler) {
			IGCFLocomotionInputHandler::Execute_HandleMoveInput(CachedLocomotionInputHandler.GetObject(), FVector2D::ZeroVector, Controller->GetControlRotation());
		}
	}
}


void UGCFLocomotionDirectionComponent::Input_MoveUp(const FInputActionValue& Value)
{
	const float UpValue = Value.Get<float>();

	if (CachedLocomotionInputHandler) {
		IGCFLocomotionInputHandler::Execute_HandleMoveUpInput(CachedLocomotionInputHandler.GetObject(), UpValue);
	}
}


/**
 * Triggered when the vertical movement input is released (ETriggerEvent::Completed).
 * Explicitly notifies the locomotion handler to halt vertical movement (e.g., flying, swimming) by passing 0.0f.
 */
void UGCFLocomotionDirectionComponent::Input_MoveUp_Completed(const FInputActionValue& Value)
{
	if (CachedLocomotionInputHandler) {
		IGCFLocomotionInputHandler::Execute_HandleMoveUpInput(CachedLocomotionInputHandler.GetObject(), 0.0f);
	}
}


FRotator UGCFLocomotionDirectionComponent::CalcMovementRotation(AController* Controller) const
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