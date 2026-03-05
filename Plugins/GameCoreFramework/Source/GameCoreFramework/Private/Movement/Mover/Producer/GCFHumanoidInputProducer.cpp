// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Movement/Mover/Producer/GCFHumanoidInputProducer.h"
#include "Movement/Mover/Input/GCFHumanoidInputs.h"
#include "Movement/Locomotion/GCFLocomotionInputProvider.h"


void UGCFHumanoidInputProducer::ProduceInput_Implementation(int32 SimTime, FMoverInputCmdContext& InputCmdResult)
{
	// Call the base class to handle standard directional movement and jumps via interface.
	Super::ProduceInput_Implementation(SimTime, InputCmdResult);

	if (CachedOwnerPawn && CachedOwnerPawn->IsLocallyControlled()) {

		if (CachedLocomotionInputProvider) {
			// Get the underlying UObject from TScriptInterface
			UObject* ProviderObj = CachedLocomotionInputProvider.GetObject();

			// --- Crouch Handling ---
			FGCFHumanoidInputs& HumanoidInputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FGCFHumanoidInputs>();
			HumanoidInputs.bWantsToCrouch = IGCFLocomotionInputProvider::Execute_GetWantsToCrouch(ProviderObj);
		}
	}
}