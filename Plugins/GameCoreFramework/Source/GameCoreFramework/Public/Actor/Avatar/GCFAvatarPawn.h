// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Actor/Pawn/GCFPawn.h"
#include "GCFAvatarPawn.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

/**
 * @brief The base avatar pawn class used by this project.
 * 
 * This class acts as a modular shell, providing a Skeletal Mesh but deferring
 * root collision components (e.g., Capsule) to derived classes.
 * It is designed to work seamlessly with the Mover plugin.
 * 
 * It implements IGCFLocomotionInputHandler and IGCFLocomotionInputProvider,
 * establishing an interface-driven architecture to securely push and pull
 * movement intents without requiring tight coupling to concrete classes.
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base humanoid pawn class used by this project."))
class AGCFAvatarPawn : public AGCFPawn
{
	GENERATED_BODY()

public:
	UE_API AGCFAvatarPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "GCF|Pawn")
	class USkeletalMeshComponent* GetSkeletalMeshComponent() const;

	UFUNCTION(BlueprintCallable, Category = "GCF|Movement")
	class UGCFCharacterMoverComponent* GetCharacterMoverComponent() const;

	//~AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~IGCFLocomotionInputHandler Interface
	UE_API virtual void HandleJumpInput_Implementation(bool bIsPressed) override;
	//~End of IGCFLocomotionInputHandler Interface

	//~IGCFLocomotionInputProvider Interface
	UE_API virtual bool GetIsJumpPressed_Implementation() const override;
	UE_API virtual bool GetIsJumpJustPressed_Implementation() const override;
	UE_API virtual void ConsumeJumpJustPressed_Implementation() override;
	//~End of IGCFLocomotionInputProvider Interface

protected:
	static const FName AvatarCollisionComponentName;
	static const FName AvatarMeshComponentName;

protected:
	// --- Cached Input Intents ---
	/** True while the jump button is being held down. */
	bool bIsJumpPressed = false;

	/** True only on the exact frame the jump button was initially pressed. */
	bool bIsJumpJustPressed = false;
};

#undef UE_API
