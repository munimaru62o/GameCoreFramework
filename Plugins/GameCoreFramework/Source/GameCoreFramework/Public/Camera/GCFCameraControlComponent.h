// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "GCFShared.h"
#include "Movement/GCFMovementTypes.h"
#include "Messages/GCFMessageTypes.h"
#include "GCFCameraControlComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UGCFCameraComponent;
class UGCFInputComponent;
class IGCFPawnDataProvider;
struct FInputActionValue;
struct FGCFPolicyChangedMovementRotationMessage;

/**
 * @brief Manages camera control input and rotation policies for the PlayerController.
 *
 * [Responsibilities]
 * 1. Consumes raw input (Mouse/Stick) and applies sensitivity/inversion.
 * 2. Feeds rotation input to the Pawn (AddControllerYawInput).
 * 3. Listens for "Rotation Policy" changes broadcasted by the active CameraMode.
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = PlayerController, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision, Networking, Replication), meta = (BlueprintSpawnableComponent, CollapseCategories))
class UGCFCameraControlComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UE_API UGCFCameraControlComponent(const FObjectInitializer& ObjectInitializer);

	/** Registers a delegate to observe rotation policy changes (FreeLook vs Locked). */
	UE_API FDelegateHandle RegisterAndExecuteDelegate(const FGCFOnMovementRotationPolicyChanged::FDelegate& Delegate, bool bExecuteImmediately = true);
	UE_API void RemoveDelegate(const FDelegateHandle& Handle);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Handler for GameplayMessages regarding policy changes. */
	void OnCameraModeMessageReceived(FGameplayTag Channel, const FGCFPolicyChangedMovementRotationMessage& Message);

	/** Binds Input Actions (Look, Zoom) via the GCF Input System. */
	TArray<FGCFBindingReceipt> HandleInputBinding(UGCFInputComponent* InputComponent, TScriptInterface<IGCFInputConfigProvider> Provider);

	/** Helper to find the CameraComponent on the controlled Pawn. */
	class UGCFCameraComponent* GetTargetCameraComponent() const;

	// --- Input Actions ---
	void Input_Zoom(const FInputActionValue& Value);
	void Input_LookMouse(const FInputActionValue& InputActionValue);
	void Input_LookStick(const FInputActionValue& InputActionValue);

	/** Applies sensitivity, inversion, and multipliers to raw input. */
	float CalculateFinalYawInput(float RawValue, EGCFInputDeviceType DeviceType);
	float CalculateFinalPitchInput(float RawValue, EGCFInputDeviceType DeviceType);

private:
	EGCFMovementRotationPolicy MovemetRotationPolicy = EGCFMovementRotationPolicy::CameraDriven;

	TUniquePtr<FGCFMessageSubscription> MessageHandle;

	FGCFOnMovementRotationPolicyChanged OnRotationPolicyChanged;
};

#undef UE_API