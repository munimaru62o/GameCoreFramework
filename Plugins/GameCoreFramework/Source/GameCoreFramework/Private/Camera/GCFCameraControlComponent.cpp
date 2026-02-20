// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Camera/GCFCameraControlComponent.h"

#include "GCFShared.h"
#include "Camera/GCFCameraComponent.h"
#include "GameFramework/Pawn.h"
#include "Input/GCFInputConfig.h"
#include "Input/GCFInputComponent.h"
#include "Input/GCFInputConfigProvider.h"
#include "EnhancedInputComponent.h"
#include "Actor/Data/GCFPawnData.h"
#include "Messages/GCFGameplayMessages.h"

namespace
{
// Base rates for stick movement (Degrees per second)
static const float LookYawRate = 300.0f;
static const float LookPitchRate = 165.0f;
}


UGCFCameraControlComponent::UGCFCameraControlComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


FDelegateHandle UGCFCameraControlComponent::RegisterAndExecuteDelegate(const FGCFOnMovementRotationPolicyChanged::FDelegate& Delegate, bool bExecuteImmediately)
{
	if (bExecuteImmediately) {
		Delegate.ExecuteIfBound(MovemetRotationPolicy);
	}
	return OnRotationPolicyChanged.Add(Delegate);
}


void UGCFCameraControlComponent::RemoveDelegate(const FDelegateHandle& Handle)
{
	OnRotationPolicyChanged.Remove(Handle);
}


void UGCFCameraControlComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AController* Controller = GetController<AController>()) {
		// Only relevant for Local Players (Human Input).
		// AI or Server-side controllers do not need input context management via this component.
		if (!Controller->IsLocalPlayerController()) {
			Deactivate();
			return;
		}
	}

	// Auto-register input bindings using the shared macro system
	GCF_REGISTER_INPUT_BINDING(this, &ThisClass::HandleInputBinding);

	// Subscribe to Policy Change messages
	if (UWorld* World = GetWorld()) {
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(World);
		MessageHandle = MakeUnique<FGCFMessageSubscription>(
			World,
			MessageSubsystem.RegisterListener<FGCFPolicyChangedMovementRotationMessage>(
			GCFGameplayTags::Message_PolicyChange_MovementRotation,
			this,
			&ThisClass::OnCameraModeMessageReceived));
	}
}


void UGCFCameraControlComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	MessageHandle.Reset();
	Super::EndPlay(EndPlayReason);
}


void UGCFCameraControlComponent::OnCameraModeMessageReceived(FGameplayTag Channel, const FGCFPolicyChangedMovementRotationMessage& Message)
{
	// Ensure the message is meant for this controller
	if (Message.Controller == GetController<AController>()) {
		if (MovemetRotationPolicy != Message.NewPolicy) {
			MovemetRotationPolicy = Message.NewPolicy;
			OnRotationPolicyChanged.Broadcast(MovemetRotationPolicy);
		}
	}
}


TArray<FGCFBindingReceipt> UGCFCameraControlComponent::HandleInputBinding(UGCFInputComponent* InputComponent, TScriptInterface<IGCFInputConfigProvider> Provider)
{
	TArray<FGCFBindingReceipt> Receipts;

	if (InputComponent && Provider) {

		for(const UGCFInputConfig* InputConfig : Provider->GetInputConfigList()) {
			FGCFInputBinder InputBinder(InputComponent, InputConfig, Receipts);

			// Bind Actions
			InputBinder.Bind(GCFGameplayTags::InputTag_Camera_Zoom, ETriggerEvent::Triggered, this, &ThisClass::Input_Zoom);
			InputBinder.Bind(GCFGameplayTags::InputTag_Look_Mouse,	ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse);
			InputBinder.Bind(GCFGameplayTags::InputTag_Look_Stick,	ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick);
		}
	}
	return Receipts;
}


UGCFCameraComponent* UGCFCameraControlComponent::GetTargetCameraComponent() const
{
	const APawn* ControlledPawn = GetPawn<APawn>();
	return ControlledPawn ? ControlledPawn->FindComponentByClass<UGCFCameraComponent>() : nullptr;
}


void UGCFCameraControlComponent::Input_Zoom(const FInputActionValue& Value)
{
	const float ZoomDelta = Value.Get<float>();

	if (UGCFCameraComponent* CameraComp = GetTargetCameraComponent()) {

		// Hardcoded sensitivity for now, could be moved to settings
		static const float ZoomSensitivity = 0.1f;
		CameraComp->AddZoomRatio(ZoomDelta * -ZoomSensitivity);
	}
}


void UGCFCameraControlComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn) {
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f) {
		Pawn->AddControllerYawInput(CalculateFinalYawInput(Value.X, EGCFInputDeviceType::Mouse));
	}

	if (Value.Y != 0.0f) {
		Pawn->AddControllerPitchInput(CalculateFinalPitchInput(Value.Y, EGCFInputDeviceType::Mouse));
	}
}


void UGCFCameraControlComponent::Input_LookStick(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn) {
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();
	const UWorld* World = GetWorld();
	check(World);

	const float DeltaTime = World->GetDeltaSeconds();

	if (Value.X != 0.0f) {
		const float ScaledValue = Value.X * DeltaTime * LookYawRate;
		Pawn->AddControllerYawInput(CalculateFinalYawInput(ScaledValue, EGCFInputDeviceType::GamePad));
	}

	if (Value.Y != 0.0f) {
		const float ScaledValue = Value.Y * DeltaTime * LookPitchRate;
		Pawn->AddControllerPitchInput(CalculateFinalPitchInput(ScaledValue, EGCFInputDeviceType::GamePad));
	}
}


float UGCFCameraControlComponent::CalculateFinalYawInput(float RawValue, EGCFInputDeviceType DeviceType)
{
	// TODO: Retrieve from GameUserSettings
	const float UserSensitivity = 0.2f;
	const float DeviceSensitivity = 1.0f;
	const float ContextMultiplier = 1.0f;

	return RawValue * UserSensitivity * DeviceSensitivity * ContextMultiplier;
}


float UGCFCameraControlComponent::CalculateFinalPitchInput(float RawValue, EGCFInputDeviceType DeviceType)
{
	// TODO: Retrieve from GameUserSettings
	const float UserSensitivity = 0.2f;
	const float DeviceSensitivity = 1.0f;
	const float ContextMultiplier = 1.0f;

	const bool bIsReverce = true;
	const float ReverceMultiplier = bIsReverce ? -1.0f : 1.0f;

	return RawValue * UserSensitivity * DeviceSensitivity * ContextMultiplier * ReverceMultiplier;
}