// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 62o. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GCFCameraModeStack.generated.h"

class UGCFCameraMode;
struct FGCFCameraModeView;
struct FGameplayTag;

/**
 * @brief Manages a stack of active camera modes and handles blending between them.
 *
 * [How it works]
 * - Modes are pushed onto the stack (index 0 is top/active).
 * - Each frame, the stack is evaluated from bottom to top.
 * - Modes can have transition times, allowing smooth blending.
 * - Modes that become fully obscured (weight 1.0 above them) are automatically removed.
 */
UCLASS()
class GAMECOREFRAMEWORK_API UGCFCameraModeStack : public UObject
{
	GENERATED_BODY()

public:
	UGCFCameraModeStack();

	void ActivateStack();
	void DeactivateStack();
	bool IsStackActive() const { return bIsActive; }

	/**
	 * Adds a new camera mode to the top of the stack.
	 * If the mode already exists in the stack, it is moved to the top and its weight is preserved.
	 */
	void PushCameraMode(TSubclassOf<UGCFCameraMode> CameraModeClass);

	/**
	 * Updates all active modes and blends their results.
	 * @return True if the stack is active and evaluated successfully.
	 */
	bool EvaluateStack(float DeltaTime, FGCFCameraModeView& OutCameraModeView);

	void DrawDebug(UCanvas* Canvas) const;

	/** Gets the blend weight and tag of the top-most layer. */
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

	/** Returns the instance of the camera mode currently at the top of the stack. */
	UGCFCameraMode* GetTopStackMode() const;

	/** Broadcasts whenever a mode is added or removed from the stack. */
	FSimpleMulticastDelegate OnStackChanged;

protected:
	/** Finds or creates a camera mode instance for the given class. */
	UGCFCameraMode* GetCameraModeInstance(TSubclassOf<UGCFCameraMode> CameraModeClass);

	/** Updates blend weights and removes obscured modes. */
	void UpdateStack(float DeltaTime);

	/** Blends the stack from bottom up. */
	void BlendStack(FGCFCameraModeView& OutCameraModeView) const;

protected:
	bool bIsActive;

	/** Pool of instantiated camera modes. Reused to avoid GC churn. */
	UPROPERTY()
	TArray<TObjectPtr<UGCFCameraMode>> CameraModeInstances;

	/** The active stack. Index 0 is the Top (most active) mode. */
	UPROPERTY()
	TArray<TObjectPtr<UGCFCameraMode>> CameraModeStack;
};