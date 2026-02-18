// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "UObject/ScriptInterface.h"
#include "GameplayTagContainer.h"
#include "Interaction/GCFInteractionConfig.h"
#include "GameFramework/GameplayMessageSubSystem.h"
#include "Messages/GCFGameplayMessages.h"
#include "GCFShared.h"
#include "Messages/GCFMessageTypes.h"
#include "GCFInteractionComponent.generated.h"

class UGCFInteractionMode;
class IGCFInteractable;
class UGCFInputComponent;
class IGCFPawnDataProvider;

/**
 * @brief Component that manages interaction detection logic based on the active camera mode.
 *
 * [Responsibilities]
 * 1. Switches detection logic (Mode) dynamically when Camera Mode changes (via GameplayMessage).
 * 2. Finds and focuses on interactable targets every tick.
 * 3. Sends Gameplay Events to GAS when the interaction input is triggered.
 */
UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class GAMECOREFRAMEWORK_API UGCFInteractionComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UGCFInteractionComponent(const FObjectInitializer& ObjectInitializer);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Returns the currently focused actor, if any. */
	UFUNCTION(BlueprintPure, Category = "GCF|Interaction")
	AActor* GetFocusedTarget() const { return FocusedTarget; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Updates the focus state and calls interface events on old/new targets. */
	void UpdateFocusedTarget(AActor* NewTarget);

	/** Switches the active interaction mode strategy based on the camera tag. */
	void UpdateActiveCameraMode(FGameplayTag NewCameraModeTag);

	/** Handler for camera mode change messages. */
	void OnCameraModeMessageReceived(FGameplayTag Channel, const FGCFCameraModeChangedMessage& Message);

protected:
	/** Data Asset defining which InteractionMode to use for each CameraMode. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GCF|Interaction")
	TObjectPtr<UGCFInteractionConfig> InteractionConfig;

	/** Collision channel used for traces. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GCF|Interaction")
	TEnumAsByte<ECollisionChannel> InteractTraceChannel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GCF|Interaction", Meta = (Categories = "Camera"))
	FGameplayTag DefaultCameraModeTag;

	/** Whether to draw debug lines for traces. */
	UPROPERTY(EditAnywhere, Category = "GCF|Interaction|Debug")
	bool bShowDebugTrace = true;

private:
	/** Current active camera mode tag. */
	UPROPERTY(Transient, VisibleInstanceOnly, Category = "GCF|Interaction")
	FGameplayTag ActiveCameraModeTag;

	/** Cached setting for the current mode. */
	const FGCFInteractionModeSetting* CurrentActiveSetting = nullptr;

	/** Cache to prevent re-instantiating modes when switching back and forth. */
	UPROPERTY(Transient)
	TMap<TSubclassOf<UGCFInteractionMode>, TObjectPtr<UGCFInteractionMode>> ModeInstanceCache;

	/** The strategy instance currently responsible for finding targets. */
	UPROPERTY(Transient)
	TObjectPtr<UGCFInteractionMode> ActiveModeInstance;

	/** The actor currently being looked at/focused. */
	UPROPERTY(Transient)
	TObjectPtr<AActor> FocusedTarget;

	/** Handle for the gameplay message subscription. */
	TUniquePtr<FGCFMessageSubscription> CameraMessageHandle;
};