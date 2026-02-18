// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "GCFShared.h"
#include "System/Binder/GCFBooleanStateBinder.h"

class UGameFrameworkComponentManager;
class AController;

/**
 * A purely event-driven binder that listens for possession transition events on a Pawn.
 *
 * Architectural Note:
 * - This class does NOT check the current state immediately (No Fast Path).
 * - It relies on the GameFrameworkComponentManager (GFCM) to broadcast 'OnPossessed' and 'OnUnPossessed' events.
 * - Use this for logic that needs to react to the *moment* of change (e.g., playing a sound, UI notification).
 * - For checking the persistent "IsPossessed" state, rely on the GFCM Feature system or FGCFStateComposer instead.
 */
class FGCFPawnPossessionBinder final : public FGCFBooleanStateBinder
{
public:
	/**
	 * Creates a binder to listen for possession events.
	 *
	 * @param InGFCM        The GameFrameworkComponentManager instance.
	 * @param InPawn        The pawn to monitor.
	 * @param InDelegate    Callback executed when the event fires (true=Possessed, false=Unpossessed).
	 * @param bAutoActivate If true, starts listening for events immediately.
	 */
	static TUniquePtr<FGCFPawnPossessionBinder> CreateBinder(UGameFrameworkComponentManager* InGFCM, APawn* InPawn, FGCFBooleanStateSignature&& InDelegate, bool bAutoActivate = true)
	{
		TUniquePtr<FGCFPawnPossessionBinder> Binder(new FGCFPawnPossessionBinder(InGFCM, InPawn, MoveTemp(InDelegate)));
		if (bAutoActivate) {
			Binder->Activate();
		}
		return Binder;
	}

private:
	FGCFPawnPossessionBinder(UGameFrameworkComponentManager* InGFCM, APawn* InSpecificActor, FGCFBooleanStateSignature&& InDelegate);
};