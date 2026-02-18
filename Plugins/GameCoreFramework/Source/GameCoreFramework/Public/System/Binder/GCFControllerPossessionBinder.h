// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "GCFShared.h"
#include "System/Binder/GCFContextBinder.h"

class UGameFrameworkComponentManager;
class AController;

/**
 * A specialized binder that monitors possession changes for a specific Controller.
 *
 * Mechanism:
 * - It listens to extension events from ALL Pawns (via APawn::StaticClass).
 * - It filters these events to check if the interacting Controller matches the specific target Controller.
 *
 * Use Case:
 * - Useful for initializing UI or camera systems that need to react whenever the player's "Body" changes.
 * - Fires 'true' when the specific controller possesses a pawn.
 * - Fires 'false' when the specific controller unpossesses its pawn.
 */
class FGCFControllerPossessionBinder final : public FGCFContextBinder
{
public:
	/**
	 * Creates and activates a binder to track possession for the given controller.
	 *
	 * @param InGFCM        The GameFrameworkComponentManager instance.
	 * @param InController  The controller to monitor.
	 * @param InDelegate    The callback to execute on possession change (true=Possessed, false=Unpossessed).
	 * @param bAutoActivate If true, starts monitoring immediately.
	 */
	static TUniquePtr<FGCFControllerPossessionBinder> CreateBinder(UGameFrameworkComponentManager* InGFCM, AController* InController, FGCFBooleanStateSignature&& InDelegate, bool bAutoActivate = true)
	{
		TUniquePtr<FGCFControllerPossessionBinder> Binder(new FGCFControllerPossessionBinder(InGFCM, InController, MoveTemp(InDelegate)));
		if (bAutoActivate) {
			Binder->Activate();
		}
		return Binder;
	}

private:
	FGCFControllerPossessionBinder(UGameFrameworkComponentManager* InGFCM, AController* InController, FGCFBooleanStateSignature&& InDelegate);

	/**
	 * Checks global pawn events and filters them for the target controller.
	 *
	 * @return true if the event was relevant to this controller (Possessed/Unpossessed), false otherwise.
	 */
	virtual bool TryResolveEvent(AActor* Actor, FName EventName) override;

private:
	/** The currently possessed pawn (used to verify unpossession events). */
	TWeakObjectPtr<APawn> Pawn;

	/** The target controller to monitor. */
	TWeakObjectPtr<AController> Controller;

	/** Callback delegate. */
	FGCFBooleanStateSignature Delegate;
};