// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularPawn.h"
#include "Movement/Locomotion/GCFLocomotionInputHandler.h"
#include "Movement/Locomotion/GCFLocomotionInputProvider.h"
#include "Actor/GCFTeamAgentInterface.h"
#include "GameplayTagAssetInterface.h"
#include "GCFPawn.generated.h"

class UGCFPawnData;
class UGCFPawnExtensionComponent;
class UGCFPawnReadyStateComponent;
class UGCFCameraComponent;
class UGCFPawnInputBridgeComponent;
class UShapeComponent;
class UMeshComponent;
class UMoverComponent;

/**
 * @brief Base Pawn class for non-humanoid entities (e.g., Spectators, Drones, Vehicles).
 * 
 * Unlike AGCFCharacter, this class does not include a CapsuleComponent or CharacterMovementComponent by default.
 * It uses a simple SphereComponent as the root for collision and supports floating movement.
 * 
 * [Features]
 * - Integrated with Modular Gameplay (PawnExtensionComponent).
 * - Handles controller/player state replication lifecycle.
 * - Supports GCF Camera system.
 * - Implements Locomotion interfaces (Handler/Provider) for decoupled, architecture-agnostic intent routing.
 * - Supports both traditional FloatingPawnMovement and the new Mover plugin architecture via cached intents.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base pawn class used by this project."))
class GAMECOREFRAMEWORK_API AGCFPawn : public AModularPawn, public IGCFLocomotionInputHandler, public IGCFLocomotionInputProvider, public IGCFTeamAgentInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	AGCFPawn(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	class USphereComponent* GetSphereComponent() const;

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	class UStaticMeshComponent* GetStaticMesh() const;

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	class UShapeComponent* GetCollisionComponent() const { return CollisionComponent; }

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	class UMeshComponent* GetMeshComponent() const { return MeshComponent; };

	UFUNCTION(BlueprintCallable, Category = "GCF|Movement")
	virtual class UMoverComponent* GetMoverComponent() const { return MoverComponent; }

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	const UGCFPawnData* GetPawnData() const;

	// Helper functions to directly modify local tags.
	UFUNCTION(BlueprintCallable, Category = "GCF|Tags")
	void AddGameplayTag(const FGameplayTag& Tag);

	UFUNCTION(BlueprintCallable, Category = "GCF|Tags")
	void RemoveGameplayTag(const FGameplayTag& Tag);

	//~APawn interface
	virtual FVector GetPawnViewLocation() const override;
	//~End of APawn interface

	//~IGameplayTagAssetInterface interface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface interface

	//~IGCFLocomotionInputHandler Interface
	virtual void HandleMoveInput_Implementation(const FVector2D& InputValue, const FRotator& MovementRotation) override;
	virtual void HandleMoveUpInput_Implementation(float Value) override;
	//~End of IGCFLocomotionInputHandler Interface

	//~IGCFLocomotionInputProvider Interface
	FVector GetDesiredMovementVector_Implementation() const override;
	//~End of IGCFLocomotionInputProvider Interface

protected:
	static const FName CollisionComponentName;
	static const FName MeshComponentName;

	//~AActor / APawn Interface
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;
	//~End of AActor / APawn Interface

	//~IGCFTeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual FOnGCFTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IGCFTeamAgentInterface interface

	// Begins the death sequence for the character (disables collision, disables movement, etc...)
	UFUNCTION()
	virtual void OnDeathStarted(AActor* OwningActor);

	// Ends the death sequence for the character (detaches controller, destroys pawn, etc...)
	UFUNCTION()
	virtual void OnDeathFinished(AActor* OwningActor);

private:
	/** Calculates the final movement vector based on current inputs and rotation, storing it for the Mover Producer. */
	void UpdateCachedTargetMovement();

	UFUNCTION()
	void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	// Called to determine what happens to the team ID when possession ends
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UShapeComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Pawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMoverComponent> MoverComponent;

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

	/** 
	 * A lightweight container to manage state tags locally without requiring a full Ability System Component.
	 * Can be overridden in child classes (e.g. AGCFPawnWithAbilities) to defer to an ASC.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Tags")
	FGameplayTagContainer PawnTags;

private:
	/** The pre-calculated movement vector, requested every tick by the Mover Input Producer. */
	FVector CachedTargetMovement = FVector::ZeroVector;

	FRotator CachedMoveRotation = FRotator::ZeroRotator;
	FVector2D CachedMoveInput = FVector2D::ZeroVector;
	float CachedUpInput = 0.0f;
};
