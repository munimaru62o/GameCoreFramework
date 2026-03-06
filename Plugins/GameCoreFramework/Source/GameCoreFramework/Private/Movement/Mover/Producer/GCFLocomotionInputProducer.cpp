// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Movement/Mover/Producer/GCFLocomotionInputProducer.h"
#include "Movement/Locomotion/GCFLocomotionInputProvider.h"
#include "GCFShared.h"


void UGCFLocomotionInputProducer::BeginPlay()
{
	Super::BeginPlay();

	// Cache the owning Pawn
	CachedOwnerPawn = Cast<APawn>(GetOwner());

	// Cache the interface to eliminate the costly Implements<U...>() search loop in the Hot Path.
	// We deliberately use TScriptInterface here to preserve Blueprint extensibility (Execute_ routing).
	// 
	// [Optimization NOTE for Production]
	// If your project requires extreme performance and you guarantee that this interface 
	// is ONLY implemented in native C++ (no Blueprint overrides), you can cast to the native 
	// pointer (IGCFLocomotionInputProvider*) and call the _Implementation functions directly.
	// This will completely bypass the VM routing overhead, but it will silently break any 
	// Blueprint overrides.
	if (CachedOwnerPawn) {
		// Assigning to a TScriptInterface automatically validates if the interface is implemented.
		// If the underlying object does not implement it, this will safely resolve to nullptr.
		CachedLocomotionInputProvider = CachedOwnerPawn;

#if !UE_BUILD_SHIPPING
		if (!CachedLocomotionInputProvider) {
			UE_LOG(LogGCFCommon, Warning, TEXT("[%s] The owning Pawn [%s] does not implement IGCFLocomotionInputProvider! Producer input will be ignored."),
				   *GetName(), *GetNameSafe(CachedOwnerPawn));
		}
#endif
	}
}


void UGCFLocomotionInputProducer::ProduceInput_Implementation(int32 SimTime, FMoverInputCmdContext& InputCmdResult)
{
	if (!CachedOwnerPawn) {
		return;
	}

	// Retrieve or create the standard character input buffer for this simulation frame.
	FCharacterDefaultInputs& InputData = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FCharacterDefaultInputs>();
	FVector DesiredMove = FVector::ZeroVector;
	FVector DesiredOrientation = CachedOwnerPawn->GetActorForwardVector();

	// We only poll input from locally controlled pawns. 
	// Simulated proxies will have their inputs populated via Network Prediction replication.
	if (CachedOwnerPawn->IsLocallyControlled()) {

		// Extract the cached movement vector (calculated from Enhanced Input / Gameplay Tags)
		// securely via the provider interface, decoupling the producer from the specific Pawn class.
		if (CachedLocomotionInputProvider) {
			// Get the underlying UObject from TScriptInterface
			UObject* ProviderObj = CachedLocomotionInputProvider.GetObject();

			// --- Movement Handling ---
			DesiredMove = IGCFLocomotionInputProvider::Execute_GetMovementIntent(ProviderObj);

			// --- Orientation Handling ---
			DesiredOrientation = IGCFLocomotionInputProvider::Execute_GetOrientationIntent(ProviderObj);

			// --- Jump Handling ---
			InputData.bIsJumpPressed = IGCFLocomotionInputProvider::Execute_GetIsJumpPressed(ProviderObj);
			InputData.bIsJumpJustPressed = IGCFLocomotionInputProvider::Execute_GetIsJumpJustPressed(ProviderObj);

			if (InputData.bIsJumpJustPressed) {
				IGCFLocomotionInputProvider::Execute_ConsumeJumpJustPressed(ProviderObj);
			}
		}
	}
	// Inject the final directional intent into Mover's input buffer.
	InputData.SetMoveInput(EMoveInputType::DirectionalIntent, DesiredMove);
	InputData.OrientationIntent = DesiredOrientation;
}