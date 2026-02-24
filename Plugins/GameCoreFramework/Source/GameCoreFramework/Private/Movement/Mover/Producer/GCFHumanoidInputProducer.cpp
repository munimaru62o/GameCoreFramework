// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Movement/Mover/Producer/GCFHumanoidInputProducer.h"
#include "Actor/Character/GCFHumanoidPawn.h"


void UGCFHumanoidInputProducer::ProduceInput_Implementation(int32 SimTime, FMoverInputCmdContext& InputCmdResult)
{
	// Call the base class to handle standard directional movement (Vector generation).
	Super::ProduceInput_Implementation(SimTime, InputCmdResult);

	if (AGCFHumanoidPawn* HumanoidPawn = Cast<AGCFHumanoidPawn>(GetOwner())) {

		if (HumanoidPawn->IsLocallyControlled()) {
			FCharacterDefaultInputs& DefaultInputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FCharacterDefaultInputs>();

			// --- Jump Handling ---
			const bool bWantsToJump = HumanoidPawn->GetWantsToJump();
			DefaultInputs.bIsJumpPressed = bWantsToJump;
			DefaultInputs.bIsJumpJustPressed = bWantsToJump;

			if (bWantsToJump) {
				HumanoidPawn->ConsumeJumpInput();
			}

			// --- Crouch Handling ---
			/* TODO: Implement crouch state injection when FGCFCharacterInputs struct is ready.
			FGCFCharacterInputs& GCFInputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FGCFCharacterInputs>();
			GCFInputs.bIsCrouchPressed = HumanoidPawn->GetWantsToCrouch();
			*/
		}
	}
}