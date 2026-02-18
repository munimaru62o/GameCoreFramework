// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "GCFMovementTypes.generated.h"

/**
 * Defines how the Character's rotation is handled relative to the camera/input.
 */
UENUM(BlueprintType)
enum class EGCFMovementRotationPolicy : uint8
{
	Invalid = 0 UMETA(Hidden),
	FreeLook,     // Character rotates independently of the camera (e.g., Adventure games).
	CameraDriven, // Character always faces the camera direction (e.g., Strafe/Shooter).
	Fixed,        // Rotation is locked or driven by external script.
};

DECLARE_MULTICAST_DELEGATE_OneParam(FGCFOnMovementRotationPolicyChanged, EGCFMovementRotationPolicy);