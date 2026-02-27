// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Movement/Mover/Mode/GCFFlyingMode.h"
#include "MoverComponent.h"
#include "MoverDataModelTypes.h"


void UGCFFlyingMode::GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	Super::GenerateMove_Implementation(StartState, TimeStep, OutProposedMove);
}