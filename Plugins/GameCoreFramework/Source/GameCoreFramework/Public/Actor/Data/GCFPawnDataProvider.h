// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GCFPawnDataProvider.generated.h"

class UGCFPawnData;

UINTERFACE(MinimalAPI)
class UGCFPawnDataProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief Interface to abstract the retrieval of Pawn Data assets.
 * * Allows systems to query "What data drives this Pawn?" without knowing
 * the exact component (e.g., PawnExtensionComponent) that holds the data.
 */
class GAMECOREFRAMEWORK_API IGCFPawnDataProvider
{
	GENERATED_BODY()

public:
	/**
	 * Helper template to get the PawnData cast to a specific type.
	 */
	template <typename T>
	const T* GetPawnData() const
	{
		return Cast<T>(GetPawnDataInternal());
	}

protected:
	/**
	 * Internal accessor for the raw PawnData.
	 * Must be implemented by the provider component.
	 */
	virtual const UGCFPawnData* GetPawnDataInternal() const = 0;
};