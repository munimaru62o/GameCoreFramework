// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "GCFShared.h"
#include "System/Binder/GCFContextBinder.h"

class UGameFrameworkComponentManager;
struct FComponentRequestHandle;


/**
 * A specialized context binder that maps a pair of extension events (On/Off) to a single boolean state change.
 *
 * Use Case:
 * - Useful for tracking binary states such as "Possessed/Unpossessed" or "Ready/NotReady".
 * - When the 'On' event fires, the delegate executes with 'true'.
 * - When the 'Off' event fires, the delegate executes with 'false'.
 */
class FGCFBooleanStateBinder : public FGCFContextBinder
{
protected:
	/**
	 * Protected constructor for derived classes.
	 *
	 * @param InGFCM          The GameFrameworkComponentManager instance.
	 * @param InReceiverClass The class of actors to listen to.
	 * @param InSpecificActor (Optional) If set, only events from this specific actor will be processed.
	 * @param InOnEvent       The event name that signifies the state becoming 'Active' (true).
	 * @param InOffEvent      The event name that signifies the state becoming 'Inactive' (false).
	 * @param InDelegate      The callback to execute when the state changes.
	 */
	FGCFBooleanStateBinder(UGameFrameworkComponentManager* InGFCM, TSoftClassPtr<AActor> InReceiverClass, AActor* InSpecificActor, FName InOnEvent, FName InOffEvent, FGCFBooleanStateSignature&& InDelegate)
		: FGCFContextBinder(InGFCM, InReceiverClass)
		, SpecificActor(InSpecificActor)
		, OnEvent(InOnEvent)
		, OffEvent(InOffEvent)
		, Delegate(MoveTemp(InDelegate))
	{};

	/**
	 * Checks if the triggered event matches either the On or Off event and executes the delegate.
	 *
	 * @param Actor     The actor that triggered the event.
	 * @param EventName The name of the event triggered.
	 * @return true if the event was handled (matched On or Off), false otherwise.
	 */
	virtual bool TryResolveEvent(AActor* Actor, FName EventName) override;

protected:
	/** Target actor to filter events. If null, all actors of the ReceiverClass are processed. */
	TWeakObjectPtr<AActor> SpecificActor;

	/** Event name that triggers the delegate with 'true'. */
	FName OnEvent;

	/** Event name that triggers the delegate with 'false'. */
	FName OffEvent;

	/** Callback to execute when a matching event is received. */
	FGCFBooleanStateSignature Delegate;
};