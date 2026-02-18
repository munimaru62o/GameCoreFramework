// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Movement/GCFMovementTypes.h"
#include "GCFGameplayMessages.generated.h"


class AController;

/**
 * @brief Message struct broadcast via GameplayMessageSubsystem when the active camera mode changes.
 */
USTRUCT(BlueprintType)
struct FGCFCameraModeChangedMessage
{
	GENERATED_BODY()

	/** The GameplayTag of the newly applied Camera Mode. */
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	FGameplayTag NewCameraModeTag;

	/** The controller associated with this camera change. Used for filtering. */
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	TObjectPtr<AController> Controller;
};


/**
 * @brief Message broadcast when the cursor visibility policy changes.
 */
USTRUCT(BlueprintType)
struct FGCFPolicyChangedCursorMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	bool bShowCursor = false;

	/** The controller associated with this policy change. */
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	TObjectPtr<AController> Controller;
};


/**
 * @brief Message broadcast when the movement rotation policy (e.g., VelocityDirection vs CameraDirection) changes.
 */
USTRUCT(BlueprintType)
struct FGCFPolicyChangedMovementRotationMessage
{
	GENERATED_BODY()

	/** The new rotation policy being applied. */
	UPROPERTY(BlueprintReadOnly, Category = "Rotation")
	EGCFMovementRotationPolicy NewPolicy = EGCFMovementRotationPolicy::Invalid;

	/** The controller associated with this policy change. */
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	TObjectPtr<AController> Controller;
};