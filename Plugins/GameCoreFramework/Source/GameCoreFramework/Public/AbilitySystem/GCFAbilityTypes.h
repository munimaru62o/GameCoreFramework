// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
//#include "GCFAbilityTypes.generated.h"


enum class EGCFAbilityContextState : uint8
{
	None			= 0,
	AbilitySystem	= 1 << 0,
	PawnReady		= 1 << 1,
	Possessed		= 1 << 2,
};
ENUM_CLASS_FLAGS(EGCFAbilityContextState);


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAbilityContextEvaluatedNative, EGCFAbilityContextState /* CurrentState */, bool /* bCanGrantAbilities */);