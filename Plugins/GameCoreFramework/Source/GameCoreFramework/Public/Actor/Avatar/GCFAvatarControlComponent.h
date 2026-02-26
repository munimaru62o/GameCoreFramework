// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Input/GCFInputTypes.h"
#include "System/Lifecycle/GCFStateTypes.h"
#include "Components/PawnComponent.h"
#include "System/Binder/GCFContextBinder.h"
#include "GCFAvatarControlComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class IGCFInputConfigProvider;
class UGCFInputComponent;
struct FInputActionValue;

/**
 * @brief Component that handles avatar-specific discrete action inputs (e.g., Jump, Crouch).
 *
 * [Responsibilities]
 * 1. Waits for the Pawn to be fully initialized (Possessed + GameplayReady).
 * 2. Binds Input Actions securely via the GCF Input System.
 * 3. Forwards boolean commands to the owning Pawn via the IGCFAvatarActionHandler interface,
 * completely decoupling the controller from any specific Pawn class implementation.
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = Pawn, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision, Networking, Replication), meta = (BlueprintSpawnableComponent, CollapseCategories))
class UGCFAvatarControlComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UE_API UGCFAvatarControlComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Checks if the Pawn is ready to receive input, then registers bindings. */
	void HandlePawnReadyStateChanged(const FGCFPawnReadyStateSnapshot& Snapshot);

	/** Registers input actions (Jump, Crouch) to the InputComponent. */
	TArray<FGCFBindingReceipt> HandleInputBinding(UGCFInputComponent* InputComponent, TScriptInterface<IGCFInputConfigProvider> Provider);

	// --- Input Handlers ---
	void Input_Jump(const FInputActionValue& InputActionValue);
	void Input_Crouch(const FInputActionValue& InputActionValue);

private:
	/** Binder to observe Pawn readiness. */
	TUniquePtr<FGCFContextBinder> Binder;

	EGCFPawnReadyState CachedPawnReadyState;
};

#undef UE_API