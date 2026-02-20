// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Camera/GCFCameraFunctionLibrary.h"

#include "GCFShared.h"
#include "Camera/GCFCameraControlComponent.h"
#include "GameFramework/Controller.h"


TUniquePtr<FGCFDelegateHandle> UGCFCameraFunctionLibrary::BindRotationPolicyScoped(AController* Controller, const FGCFOnMovementRotationPolicyChanged::FDelegate& Delegate, bool bExecuteImmediately)
{
	// Find the component responsible for rotation policy
	UGCFCameraControlComponent* CameraComp = Controller ? Controller->FindComponentByClass<UGCFCameraControlComponent>() : nullptr;

	return FGCFDelegateHandle::CreateScoped(
		CameraComp,
		bExecuteImmediately,
		&UGCFCameraControlComponent::RegisterAndExecuteDelegate,
		&UGCFCameraControlComponent::RemoveDelegate,
		Delegate);
}