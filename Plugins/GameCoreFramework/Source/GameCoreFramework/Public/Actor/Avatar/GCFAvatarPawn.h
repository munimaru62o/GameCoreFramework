// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Actor/Pawn/GCFPawn.h"
#include "Actor/Avatar/GCFAvatarActionHandler.h"
#include "Actor/Avatar/GCFAvatarActionProvider.h"
#include "GCFAvatarPawn.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UGCFAvatarControlComponent;
enum class EStanceMode : uint8;

/**
 * @brief The base avatar pawn class used by this project.
 * 
 * This class acts as a modular shell, providing a Capsule Collision and a Skeletal Mesh.
 * It is designed to work seamlessly with the Mover plugin. Movement logic and Input Producers
 * are expected to be attached via Blueprints (Composition over Inheritance) to maximize flexibility.
 * 
 * It implements Handler interfaces to receive inputs from the Controller, and Provider
 * interfaces to securely supply those cached intents to the Mover Input Producer.
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base humanoid pawn class used by this project."))
class AGCFAvatarPawn : public AGCFPawn, public IGCFAvatarActionHandler, public IGCFAvatarActionProvider
{
	GENERATED_BODY()

public:
	UE_API AGCFAvatarPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	class UCapsuleComponent* GetCapsuleComponent() const;

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	class USkeletalMeshComponent* GetSkeletalMeshComponent() const;

	UFUNCTION(BlueprintCallable, Category = "GCF|Movement")
	class UGCFCharacterMoverComponent* GetCharacterMoverComponent() const;

	//~AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~APawn interface
	UE_API virtual FVector GetPawnViewLocation() const override;
	//~End of APawn interface

	//~IGCFAvatarActionHandler interface
	UE_API virtual void HandleJumpInput_Implementation(bool bIsPressed) override;
	UE_API virtual void HandleCrouchInput_Implementation(bool bIsPressed) override;
	//~End of IGCFAvatarActionHandler interface

	/** Toggles the crouching state by adding/removing the corresponding Gameplay Tag. */
	UE_API void ToggleCrouch();

	//~IGCFAvatarActionProvider interface (Pull / Read from Input Producer)
	UE_API virtual bool GetIsJumpPressed_Implementation() const override;
	UE_API virtual bool GetIsJumpJustPressed_Implementation() const override;
	UE_API virtual void ConsumeJumpJustPressed_Implementation() override;
	UE_API virtual bool GetWantsToCrouch_Implementation() const override;
	//~End of IGCFAvatarActionProvider interface

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFAvatarControlComponent> AvatarControlComponent;

	// --- Cached Input Intents ---
	/** True while the jump button is being held down. */
	bool bIsJumpPressed = false;

	/** True only on the exact frame the jump button was initially pressed. */
	bool bIsJumpJustPressed = false;

	/** * Indicates whether the player currently intends to crouch.
	 * Note: This is purely an input intent, NOT the actual physical state.
	 * To check the actual state, query the MoverComponent for the "Mover.IsCrouching" tag.
	 */
	bool bWantsToCrouch = false;

	/** Tracks the physical state of the crouch button to prevent continuous toggling while held. */
	bool bIsCrouchButtonPressed = false;
};

#undef UE_API
