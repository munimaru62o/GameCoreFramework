// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.


#include "Movement/Mover/Input/GCFMoverInputsFunctionLibrary.h"
#include "MoverTypes.h"


FGCFHumanoidInputs UGCFMoverInputsFunctionLibrary::GetGCFHumanoidInputs(const FMoverDataCollection& FromCollection)
{
	if (const FGCFHumanoidInputs* FoundInputs = FromCollection.FindDataByType<FGCFHumanoidInputs>()) {
		return *FoundInputs;
	}

	return FGCFHumanoidInputs();
}