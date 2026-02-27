// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Movement/Mover/Producer/GCFCachedInputProducer.h"
#include "Movement/Locomotion/GCFLocomotionInputProvider.h"


void UGCFCachedInputProducer::ProduceInput_Implementation(int32 SimTime, FMoverInputCmdContext& InputCmdResult)
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) {
        return;
    }

	// Retrieve or create the standard character input buffer for this simulation frame.
	FCharacterDefaultInputs& InputData = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FCharacterDefaultInputs>();
	FVector DesiredMove = FVector::ZeroVector;

	// We only poll input from locally controlled pawns. 
	// Simulated proxies will have their inputs populated via Network Prediction replication.
	if (OwnerPawn->IsLocallyControlled()) {

		// Extract the cached movement vector (calculated from Enhanced Input / Gameplay Tags)
		// securely via the provider interface, decoupling the producer from the specific Pawn class.
		if (OwnerPawn->Implements<UGCFLocomotionInputProvider>()) {
			// --- Movement Handling ---
			DesiredMove = IGCFLocomotionInputProvider::Execute_GetDesiredMovementVector(OwnerPawn);

			// --- Jump Handling ---
			InputData.bIsJumpPressed = IGCFLocomotionInputProvider::Execute_GetIsJumpPressed(OwnerPawn);
			InputData.bIsJumpJustPressed = IGCFLocomotionInputProvider::Execute_GetIsJumpJustPressed(OwnerPawn);

			if (InputData.bIsJumpJustPressed) {
				IGCFLocomotionInputProvider::Execute_ConsumeJumpJustPressed(OwnerPawn);
			}
		}
	}
	// Inject the final directional intent into Mover's input buffer.
	InputData.SetMoveInput(EMoveInputType::DirectionalIntent, DesiredMove);
	InputData.OrientationIntent = OwnerPawn->GetControlRotation().Vector();
}