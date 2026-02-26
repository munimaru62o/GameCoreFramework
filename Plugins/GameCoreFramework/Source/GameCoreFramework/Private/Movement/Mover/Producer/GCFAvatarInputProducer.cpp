// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Movement/Mover/Producer/GCFAvatarInputProducer.h"
#include "Movement/Mover/Input/GCFAvatarInputs.h"
#include "Actor/Avatar/GCFAvatarPawn.h"
#include "Actor/GCFActorFunctionLibrary.h"
#include "Actor/Avatar/GCFAvatarActionProvider.h"


void UGCFAvatarInputProducer::ProduceInput_Implementation(int32 SimTime, FMoverInputCmdContext& InputCmdResult)
{
	// Call the base class to handle standard directional movement (Vector generation).
	Super::ProduceInput_Implementation(SimTime, InputCmdResult);

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner())) {

		if (OwnerPawn->IsLocallyControlled()) {
			FCharacterDefaultInputs& DefaultInputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FCharacterDefaultInputs>();

			if (TScriptInterface<IGCFAvatarActionProvider> Handler = UGCFActorFunctionLibrary::ResolveAvatarActionProvider(OwnerPawn)) {
				
				// --- Jump Handling ---
				DefaultInputs.bIsJumpPressed = IGCFAvatarActionProvider::Execute_GetIsJumpPressed(OwnerPawn);
				DefaultInputs.bIsJumpJustPressed = IGCFAvatarActionProvider::Execute_GetIsJumpJustPressed(OwnerPawn);

				if (DefaultInputs.bIsJumpJustPressed) {
					IGCFAvatarActionProvider::Execute_ConsumeJumpJustPressed(OwnerPawn);
				}

				// --- Crouch Handling ---
				FGCFAvatarInputs& AvatarInputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FGCFAvatarInputs>();
				AvatarInputs.bWantsToCrouch = IGCFAvatarActionProvider::Execute_GetWantsToCrouch(OwnerPawn);
			}
		}
	}
}