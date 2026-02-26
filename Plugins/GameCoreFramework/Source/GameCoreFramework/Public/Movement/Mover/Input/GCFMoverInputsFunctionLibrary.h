// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Movement/Mover/Input/GCFAvatarInputs.h"
#include "GCFMoverInputsFunctionLibrary.generated.h"

struct FMoverDataCollection;

/**
 * @brief Blueprint function library for extracting custom Mover input structures.
 * * Provides utility functions to safely extract specialized input data
 * (e.g., FGCFAvatarInputs) from a generic FMoverDataCollection.
 */
UCLASS()
class GAMECOREFRAMEWORK_API UGCFMoverInputsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Extracts the GCFAvatarInputs from a given Mover Data Collection.
	 * * @param FromCollection The collection to search within (e.g., InputCmd.InputCollection)
	 * @return The found inputs, or a default empty struct if not found.
	 */
	UFUNCTION(BlueprintCallable, Category = "GCF|Mover|Input")
	static FGCFAvatarInputs GetGCFAvatarInputs(const FMoverDataCollection& FromCollection);
};