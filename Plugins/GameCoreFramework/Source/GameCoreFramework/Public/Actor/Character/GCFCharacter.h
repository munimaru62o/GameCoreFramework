// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#pragma once

#include "ModularCharacter.h"
#include "AbilitySystemInterface.h"
#include "Actor/GCFTeamAgentInterface.h"
#include "Movement/GCFLocomotionHandler.h"
#include "Actor/Avatar/GCFAvatarActionHandler.h"
#include "GameplayEffect.h"
#include "GCFCharacter.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class AActor;
class AController;
class UAbilitySystemComponent;
class UInputComponent;
class UGCFHealthComponent;
class UGCFCameraComponent;
class UGCFPawnExtensionComponent;
class UGCFPawnReadyStateComponent;
class UGCFAvatarControlComponent;
struct FGameplayTag;
struct FGameplayTagContainer;


/**
 * AGCFCharacter
 *
 *	The base character pawn class used by this project.
 *	Responsible for sending events to pawn components.
 *	New behavior should be added via pawn components when possible.
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class AGCFCharacter : public AModularCharacter, public IGCFLocomotionHandler, public IGCFTeamAgentInterface, public IGCFAvatarActionHandler
{
	GENERATED_BODY()

public:

	UE_API AGCFCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void Reset() override;
	//~End of AActor interface

	//~APawn interface
	UE_API virtual void NotifyControllerChanged() override;
	UE_API virtual FVector GetPawnViewLocation() const override;
	//~End of APawn interface

	//~IGCFLocomotionHandler Interface
	virtual void HandleMoveInput_Implementation(const FVector2D& InputValue, const FRotator& MovementRotation) override;
	virtual void HandleMoveUpInput_Implementation(float Value) override;
	//~End of IGCFLocomotionHandler Interface

	//~IGCFAvatarActionHandler Interface (Push / Write from Controller)
	virtual void HandleJumpInput_Implementation(bool bIsPressed) override;
	virtual void HandleCrouchInput_Implementation(bool bIsPressed) override;
	//~End of IGCFAvatarActionHandler Interface

	//~IGCFTeamAgentInterface interface
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnGCFTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IGCFTeamAgentInterface interface

protected:

	UE_API virtual void PossessedBy(AController* NewController) override;
	UE_API virtual void UnPossessed() override;
	UE_API virtual void OnRep_Controller() override;

	UE_API virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Begins the death sequence for the character (disables collision, disables movement, etc...)
	UFUNCTION()
	UE_API virtual void OnDeathStarted(AActor* OwningActor);

	// Ends the death sequence for the character (detaches controller, destroys pawn, etc...)
	UFUNCTION()
	UE_API virtual void OnDeathFinished(AActor* OwningActor);

	UE_API void DisableMovementAndCollision();
	UE_API void DestroyDueToDeath();
	UE_API void UninitAndDestroy();

	// Called when the death sequence for the character has completed
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnDeathFinished"))
	UE_API void K2_OnDeathFinished();

	UE_API virtual bool CanJumpInternal_Implementation() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFPawnExtensionComponent> PawnExtComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFPawnReadyStateComponent> PawnReadyStateComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFAvatarControlComponent> AvatarControlComponent;

	UPROPERTY()
	FGenericTeamId MyTeamID;

	UPROPERTY()
	FOnGCFTeamIndexChangedDelegate OnTeamChangedDelegate;

	/** Tracks the physical state of the crouch button to prevent continuous toggling while held. */
	bool bIsCrouchButtonPressed = false;

protected:
	// Called to determine what happens to the team ID when possession ends
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
	{
		// This could be changed to return, e.g., OldTeamID if you want to keep it assigned afterwards, or return an ID for some neutral faction, or etc...
		return FGenericTeamId::NoTeam;
	}

private:
	UFUNCTION()
	UE_API void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);
};

#undef UE_API
