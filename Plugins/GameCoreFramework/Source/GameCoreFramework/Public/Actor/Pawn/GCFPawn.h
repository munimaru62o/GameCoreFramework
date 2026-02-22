// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularPawn.h"
#include "Actor/GCFTeamAgentInterface.h"
#include "Movement/GCFLocomotionHandler.h"
#include "Movement/GCFMovementInputProvider.h"

#include "GCFPawn.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UGCFPawnData;
class UGCFPawnExtensionComponent;
class UGCFPawnReadyStateComponent;
class UGCFCameraComponent;
class UGCFPawnInputBridgeComponent;
class UShapeComponent;
class UMeshComponent;

/**
 * @brief Base Pawn class for non-humanoid entities (e.g., Spectators, Drones, Vehicles).
 * * Unlike AGCFCharacter, this class does not include a CapsuleComponent or CharacterMovementComponent by default.
 * It uses a simple SphereComponent as the root for collision and supports floating movement.
 * * [Features]
 * - Integrated with Modular Gameplay (PawnExtensionComponent).
 * - Handles controller/player state replication lifecycle.
 * - Supports GCF Camera system.
 * - Supports both traditional FloatingPawnMovement and the new Mover plugin architecture via caching.
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base pawn class used by this project."))
class AGCFPawn : public AModularPawn, public IGCFLocomotionHandler, public IGCFMovementInputProvider, public IGCFTeamAgentInterface
{
	GENERATED_BODY()

public:
	UE_API AGCFPawn(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	class USphereComponent* GetSphereComponent() const;

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	class UStaticMeshComponent* GetStaticMesh() const;

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	class UShapeComponent* GetCollisionComponent() const { return CollisionComponent; }

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	class UMeshComponent* GetMeshComponent() const { return MeshComponent; };

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	const UGCFPawnData* GetPawnData() const;

	UE_API virtual void PostInitializeComponents() override;

protected:
	static const FName CollisionComponentName;
	static const FName MeshComponentName;

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

	//~IGCFMovementInputProvider Interface
	FVector GetDesiredMovementVector_Implementation() const override;
	//~End of IGCFMovementInputProvider Interface

	//~IGCFTeamAgentInterface interface
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnGCFTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IGCFTeamAgentInterface interface

	// Begins the death sequence for the character (disables collision, disables movement, etc...)
	UFUNCTION()
	UE_API virtual void OnDeathStarted(AActor* OwningActor);

	// Ends the death sequence for the character (detaches controller, destroys pawn, etc...)
	UFUNCTION()
	UE_API virtual void OnDeathFinished(AActor* OwningActor);

private:
	/** Calculates the final movement vector based on current inputs and rotation, storing it for the Mover Producer. */
	void UpdateCachedTargetMovement();

	UFUNCTION()
	UE_API void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	// Called to determine what happens to the team ID when possession ends
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UShapeComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFPawnExtensionComponent> PawnExtensionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFPawnReadyStateComponent> PawnReadyStateComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFPawnInputBridgeComponent> PawnInputBridgeComponent;

	/** Determines whether this pawn routes input to a Mover component instead of traditional PawnMovement. */
	UPROPERTY(EditDefaultsOnly, Category = "GCF|Movement")
	bool bUseMoverComponent = false;

	UPROPERTY()
	FGenericTeamId MyTeamID;

	UPROPERTY()
	FOnGCFTeamIndexChangedDelegate OnTeamChangedDelegate;

private:
	/** The pre-calculated movement vector, requested every tick by the Mover Input Producer. */
	FVector CachedTargetMovement = FVector::ZeroVector;

	FRotator CachedMoveRotation = FRotator::ZeroRotator;
	FVector2D CachedMoveInput = FVector2D::ZeroVector;
	float CachedUpInput = 0.0f;
};

#undef UE_API
