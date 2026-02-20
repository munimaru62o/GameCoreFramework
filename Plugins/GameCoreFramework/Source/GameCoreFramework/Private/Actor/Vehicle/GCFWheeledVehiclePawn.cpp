// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#include "Actor/Vehicle/GCFWheeledVehiclePawn.h"
#include "Actor/Data/GCFPawnData.h"
#include "System/Lifecycle/GCFPawnExtensionComponent.h"
#include "Camera/GCFCameraComponent.h"
#include "Components/SphereComponent.h"
#include "System/Lifecycle/GCFPawnReadyStateComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "Actor/Vehicle/GCFVehicleControlComponent.h"
#include "Input/GCFPawnInputBridgeComponent.h"
#include "Net/UnrealNetwork.h"


AGCFWheeledVehiclePawn::AGCFWheeledVehiclePawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	PawnExtensionComponent = CreateDefaultSubobject<UGCFPawnExtensionComponent>(TEXT("GCFPawnExtensionComponent"));
	PawnReadyStateComponent = CreateDefaultSubobject<UGCFPawnReadyStateComponent>(TEXT("GCFPawnReadyStateComponent"));
	VehicleControlComponent = CreateDefaultSubobject<UGCFVehicleControlComponent>(TEXT("GCFVehicleControlComponent"));
	PawnInputBridgeComponent = CreateDefaultSubobject<UGCFPawnInputBridgeComponent>(TEXT("GCFPawnInputBridgeComponent"));

	CameraComponent = CreateDefaultSubobject<UGCFCameraComponent>(TEXT("GCFCameraComponent"));
	CameraComponent->SetupAttachment(GetMesh());
	CameraComponent->bUsePawnControlRotation = false; // Camera rotation is handled by the CameraComponent/Modes.
}


void AGCFWheeledVehiclePawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGCFWheeledVehiclePawn, bIsHeadLightTurnOn);
}


void AGCFWheeledVehiclePawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Ensure the vehicle starts in a parked state.
	SetHandBrake(true);
}


const UGCFPawnData* AGCFWheeledVehiclePawn::GetPawnData() const
{
	if (PawnExtensionComponent) {
		return PawnExtensionComponent->GetPawnData<UGCFPawnData>();
	}
	return nullptr;
}


void AGCFWheeledVehiclePawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Notify extension component on Server side possession.
	if (PawnExtensionComponent) {
		PawnExtensionComponent->OnControllerAssigned();
	}
}


void AGCFWheeledVehiclePawn::UnPossessed()
{
	Super::UnPossessed();

	// Notify extension component to clean up or reset state.
	if (PawnExtensionComponent) {
		PawnExtensionComponent->HandleControllerChanged();
	}

	// Safely halt the vehicle and engage the handbrake when the driver leaves.
	UChaosVehicleMovementComponent* VehicleMove = GetVehicleMovementComponent();
	VehicleMove->StopMovementImmediately();
	SetHandBrake(true);
}


void AGCFWheeledVehiclePawn::OnRep_Controller()
{
	Super::OnRep_Controller();

	// Notify extension component on Client side when the Controller reference replicates.
	if (PawnExtensionComponent) {
		PawnExtensionComponent->OnControllerAssigned();
	}
}


void AGCFWheeledVehiclePawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Notify extension component on Client side when the PlayerState reference replicates.
	// This is crucial for initializing AbilitySystem or HUD that depends on PlayerState data.
	if (PawnExtensionComponent) {
		PawnExtensionComponent->HandlePlayerStateReplicated();
	}
}


void AGCFWheeledVehiclePawn::HandleMoveInput_Implementation(const FVector2D& InputValue, const FRotator& MovementRotation)
{
	UChaosVehicleMovementComponent* VehicleMove = GetVehicleMovementComponent();

	// Map X-axis (W/S) to Throttle and Brake
	VehicleMove->SetThrottleInput(InputValue.X > 0 ? InputValue.X : 0.0f);
	VehicleMove->SetBrakeInput(InputValue.X < 0 ? -InputValue.X : 0.0f);

	// Map Y-axis (A/D) to Steering
	VehicleMove->SetSteeringInput(InputValue.Y);
}


void AGCFWheeledVehiclePawn::HandleMoveUpInput_Implementation(float Value)
{
}


void AGCFWheeledVehiclePawn::ToggleHandBrakeInput()
{
	SetHandBrake(!bIsHandbraking);
}


void AGCFWheeledVehiclePawn::ToggleHeadLightInput()
{
	SetHeadLightState(!bIsHeadLightTurnOn);
}


void AGCFWheeledVehiclePawn::SetHandBrake(bool bNewHandbrake)
{
	bIsHandbraking = bNewHandbrake;
	GetVehicleMovementComponent()->SetHandbrakeInput(bIsHandbraking);
}


void AGCFWheeledVehiclePawn::SetHeadLightState(bool bNewHeadLight)
{
	if (HasAuthority()) {
		if (bIsHeadLightTurnOn != bNewHeadLight) {
			bIsHeadLightTurnOn = bNewHeadLight;

			// Manually call OnRep on the server so the visual changes occur locally as well.
			OnRep_IsHeadLightTurnOn();
		}
	} else {
		// If we are a client, request the server to apply the state change.
		Server_SetHeadLightState(bNewHeadLight);
	}
}

void AGCFWheeledVehiclePawn::Server_SetHeadLightState_Implementation(bool bNewHeadLight)
{
	SetHeadLightState(bNewHeadLight);
}


void AGCFWheeledVehiclePawn::OnRep_IsHeadLightTurnOn()
{
	HandleHeadLightStateChange(bIsHeadLightTurnOn);
}


void AGCFWheeledVehiclePawn::OnDeathStarted(AActor*)
{
	// Disable collision and movement here if needed.
	if (GetMesh()) {
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}


void AGCFWheeledVehiclePawn::OnDeathFinished(AActor*)
{
	// Detach controller and destroy pawn logic.
	DetachFromControllerPendingDestroy();
	SetLifeSpan(0.1f);
}