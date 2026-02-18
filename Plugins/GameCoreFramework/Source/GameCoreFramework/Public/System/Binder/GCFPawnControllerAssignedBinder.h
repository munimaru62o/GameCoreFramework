// Cop// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "GCFShared.h"
#include "System/Binder/GCFBooleanStateBinder.h"

class UGameFrameworkComponentManager;
class AController;

/**
 * A binder that monitors a specific Pawn to detect when a Controller is assigned to it.
 *
 * Use Case:
 * - Primarily used within the Pawn Extension Component to wait for the "Soul" (Controller) to arrive.
 * - Unlike FGCFControllerPossessionBinder (which monitors from the Controller's perspective),
 * this class monitors from the Pawn's perspective.
 *
 * Note:
 * - This binder only tracks the 'Assigned' state (OnEvent). It does not track un-assignment (OffEvent is None).
 */
class FGCFPawnControllerAssignedBinder final : public FGCFBooleanStateBinder
{
public:
	/**
	 * Creates a binder for a specific Pawn instance.
	 *
	 * @param InGFCM        The GameFrameworkComponentManager instance.
	 * @param InPawn        The specific Pawn to monitor.
	 * @param InDelegate    Callback executed when a controller is assigned.
	 * @param bAutoActivate If true, attempts to resolve immediately or starts listening.
	 */
	static TUniquePtr<FGCFPawnControllerAssignedBinder> CreateBinder(UGameFrameworkComponentManager* InGFCM, APawn* InPawn, FGCFBooleanStateSignature&& InDelegate, bool bAutoActivate = true)
	{
		TUniquePtr<FGCFPawnControllerAssignedBinder> Binder(new FGCFPawnControllerAssignedBinder(InGFCM, InPawn, MoveTemp(InDelegate)));
		if (bAutoActivate) {
			Binder->Activate();
		}
		return Binder;
	}

private:
	FGCFPawnControllerAssignedBinder(UGameFrameworkComponentManager* InGFCM, APawn* InPawn, FGCFBooleanStateSignature&& InDelegate);

	/**
	 * Checks if the pawn already possesses a valid controller.
	 * @return true if the controller is already assigned, skipping event registration.
	 */
	bool TryResolveImmediate() override;
};