// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Actor/Pawn/GCFPawn.h"
#include "GCFHumanoidPawn.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UGCFCharacterControlComponent;

/**
 * @brief The base humanoid pawn class used by this project.
 * 
 * This class acts as a modular shell, providing a Capsule Collision and a Skeletal Mesh.
 * It is designed to work seamlessly with the Mover plugin. Movement logic and Input Producers
 * are expected to be attached via Blueprints (Composition over Inheritance) to maximize flexibility.
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base humanoid pawn class used by this project."))
class AGCFHumanoidPawn : public AGCFPawn
{
	GENERATED_BODY()

public:
	UE_API AGCFHumanoidPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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

	/** Toggles the crouching state by adding/removing the corresponding Gameplay Tag. */
	UE_API void ToggleCrouch();

	/** Caches the jump intent. Polled and consumed by the Mover Input Producer. */
	UE_API void Jump();
	UE_API bool CanJump() const;

	/** Polled by the Input Producer to determine if the pawn intends to crouch. */
	UE_API bool GetWantsToCrouch() const;

	/** Polled by the Input Producer to determine if the pawn intends to jump. */
	UE_API bool GetWantsToJump() const;

	/** Called by the Input Producer to clear the jump intent after it has been injected into the Mover commands. */
	UE_API void ConsumeJumpInput() { bWantsToJump = false; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFCharacterControlComponent> CharacterControlComponent;

	/** Cached jump intent flag. Handled and cleared by the Input Producer. */
	bool bWantsToJump = false;
};

#undef UE_API
