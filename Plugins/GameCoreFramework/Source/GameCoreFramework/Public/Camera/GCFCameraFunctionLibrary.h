// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Movement/GCFMovementTypes.h"
#include "GCFCameraFunctionLibrary.generated.h"

class AController;
class FGCFDelegateHandle;

#define UE_API GAMECOREFRAMEWORK_API

/**
 * @brief Static function library dedicated to Camera & Control rotation utilities.
 */
UCLASS(Abstract, MinimalAPI)
class UGCFCameraFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Binds a delegate to monitor changes in the Movement Rotation Policy (e.g., FreeLook vs CameraDriven).
	 * @param Controller           The Controller owning the CameraControlComponent.
	 * @param Delegate             Callback function.
	 * @param bExecuteImmediately  If true, fires immediately with the current policy.
	 * @return Scoped handle for auto-unbinding.
	 */
	UE_API static TUniquePtr<FGCFDelegateHandle> BindRotationPolicyScoped(AController* Controller, const FGCFOnMovementRotationPolicyChanged::FDelegate& Delegate, bool bExecuteImmediately = true);

};

#undef UE_API