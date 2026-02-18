// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 62o. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Vehicle/GCFModularVehicle.h"
#include "Movement/GCFLocomotionHandler.h"

#include "GCFWheeledVehiclePawn.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UGCFPawnData;
class USphereComponent;
class UGCFPawnExtensionComponent;
class UGCFPawnReadyStateComponent;
class UGCFCameraComponent;
class UGCFVehicleControlComponent;
class UGCFPawnInputBridgeComponent;


/**
 * @brief The base wheeled vehicle pawn class used by the GCF framework.
 *
 * Integrates Unreal's Chaos Vehicle system with GCF's modular component architecture.
 * Handles locomotion input routing, state replication (e.g., headlights), and lifecycle
 * management when drivers possess or unpossess the vehicle.
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base pawn class used by this project."))
class AGCFWheeledVehiclePawn : public AGCFModularVehicle, public IGCFLocomotionHandler
{
	GENERATED_BODY()

public:
	UE_API AGCFWheeledVehiclePawn(const FObjectInitializer& ObjectInitializer);

	UE_API virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	const UGCFPawnData* GetPawnData() const;

	/** Toggles the local handbrake state. */
	void ToggleHandBrakeInput();

	/** Requests to toggle the headlight state (handled via Server RPC if Client). */
	void ToggleHeadLightInput();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~AActor / APawn Interface
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;
	//~End of AActor / APawn Interface

	//~IGCFLocomotionHandler Interface
	virtual void HandleMoveInput_Implementation(const FVector2D& InputValue, const FRotator& MovementRotation) override;
	virtual void HandleMoveUpInput_Implementation(float Value) override;
	//~End of IGCFLocomotionHandler Interface

	/** Blueprint hook to update visual effects or lights when the headlight state changes. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "GCF|Vehicle")
	void HandleHeadLightStateChange(bool bNewHeadLight);

	// Begins the death sequence for the character (disables collision, disables movement, etc...)
	UFUNCTION()
	UE_API virtual void OnDeathStarted(AActor* OwningActor);

	// Ends the death sequence for the character (detaches controller, destroys pawn, etc...)
	UFUNCTION()
	UE_API virtual void OnDeathFinished(AActor* OwningActor);

private:
	/** Applies the handbrake to the Chaos Vehicle Movement Component. */
	void SetHandBrake(bool bNewHandbrake);

	/** Internal method to set headlight state, routing to RPC if necessary. */
	void SetHeadLightState(bool bNewHeadLight);

	/** Server RPC to validate and apply headlight state changes requested by a client. */
	UFUNCTION(Server, Reliable)
	void Server_SetHeadLightState(bool bNewHeadLight);

	UFUNCTION()
	void OnRep_IsHeadLightTurnOn();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFPawnExtensionComponent> PawnExtensionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFPawnReadyStateComponent> PawnReadyStateComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFVehicleControlComponent> VehicleControlComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFPawnInputBridgeComponent> PawnInputBridgeComponent;

private:
	/** Local tracking of the handbrake state. */
	bool bIsHandbraking = false;

	/** Networked state of the headlights. */
	UPROPERTY(ReplicatedUsing = OnRep_IsHeadLightTurnOn)
	bool bIsHeadLightTurnOn;
};

#undef UE_API
