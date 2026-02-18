// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/Composer/GCFGenericStateComposer.h"


void FGCFGenericStateComposer::Add(uint32 Bit, TSharedRef<IGCFStatePredicate> Predicate)
{
	Entries.Add({ Bit, Predicate });
}

uint32 FGCFGenericStateComposer::Compute() const
{
	uint32 Result = 0;

	// Iterate over all registered rules
	for (const FEntry& Entry : Entries) {
		// If the condition is met, activate the corresponding bit flag.
		if (Entry.Predicate->Evaluate()) {
			Result |= Entry.Bit;
		}
	}

	return Result;
}
