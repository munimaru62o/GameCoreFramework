// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.


#include "Movement/Mover/Input/GCFMoverInputsFunctionLibrary.h"
#include "MoverTypes.h"


FGCFAvatarInputs UGCFMoverInputsFunctionLibrary::GetGCFAvatarInputs(const FMoverDataCollection& FromCollection)
{
	if (const FGCFAvatarInputs* FoundInputs = FromCollection.FindDataByType<FGCFAvatarInputs>()) {
		return *FoundInputs;
	}

	return FGCFAvatarInputs();
}