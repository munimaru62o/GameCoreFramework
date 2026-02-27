// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Movement/Mover/Producer/GCFHumanoidInputProducer.h"
#include "Movement/Mover/Input/GCFHumanoidInputs.h"
#include "Actor/GCFActorFunctionLibrary.h"
#include "Actor/Humanoid/GCFHumanoid.h"


void UGCFHumanoidInputProducer::ProduceInput_Implementation(int32 SimTime, FMoverInputCmdContext& InputCmdResult)
{
	// Call the base class to handle standard directional movement (Vector generation).
	Super::ProduceInput_Implementation(SimTime, InputCmdResult);

	if (AGCFHumanoid* Humanoid = Cast<AGCFHumanoid>(GetOwner())) {
		if (Humanoid->IsLocallyControlled()) {
			// --- Crouch Handling ---
			FGCFHumanoidInputs& HumanoidInputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FGCFHumanoidInputs>();
			HumanoidInputs.bWantsToCrouch  = Humanoid->GetWantsToCrouch();
		}
	}
}