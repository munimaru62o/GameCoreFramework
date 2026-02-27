// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Actor/Pawn/GCFPawn.h"
#include "GCFAvatarPawn.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UGCFAvatarControlComponent;
enum class EStanceMode : uint8;

/**
 * @brief The base avatar pawn class used by this project.
 * 
 * This class acts as a modular shell, providing a Skeletal Mesh but deferring
 * root collision components (e.g., Capsule) to derived classes.
 * It is designed to work seamlessly with the Mover plugin.
 * 
 * It provides direct API methods for Control Components (to push input intents)
 * and Mover Input Producers (to pull cached intents), ensuring strict type safety
 * and optimal performance through direct casting.
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

	// --- Input Handlers (Push / Write from Control Component) ---
	UE_API void HandleJumpInput(bool bIsPressed);

	// --- Input Providers (Pull / Read from Input Producer) ---
	UE_API bool GetIsJumpPressed() const;
	UE_API bool GetIsJumpJustPressed() const;
	UE_API void ConsumeJumpJustPressed();

protected:
	static const FName AvatarCollisionComponentName;
	static const FName AvatarMeshComponentName;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFAvatarControlComponent> AvatarControlComponent;

	// --- Cached Input Intents ---
	/** True while the jump button is being held down. */
	bool bIsJumpPressed = false;

	/** True only on the exact frame the jump button was initially pressed. */
	bool bIsJumpJustPressed = false;
};

#undef UE_API
