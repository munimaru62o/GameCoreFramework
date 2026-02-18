// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.


#include "System/Binder/GCFBooleanStateBinder.h"


bool FGCFBooleanStateBinder::TryResolveEvent(AActor* Actor, FName EventName)
{
	// Filter by specific actor if one is assigned
	if (SpecificActor.Get() && Actor != SpecificActor) {
		return false;
	}

	if (EventName == OnEvent) {
		Delegate.ExecuteIfBound(Actor, true);
		return true;
	}
	if (EventName == OffEvent) {
		Delegate.ExecuteIfBound(Actor, false);
		return true;
	}
	return false;
}