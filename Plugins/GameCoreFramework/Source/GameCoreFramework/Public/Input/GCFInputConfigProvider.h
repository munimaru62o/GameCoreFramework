// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "GCFShared.h"
#include "UObject/Interface.h"
#include "GCFInputConfigProvider.generated.h"

class UGCFInputConfig;


UINTERFACE(MinimalAPI)
class UGCFInputConfigProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface that defines the contract for any entity capable of providing Input Configurations.
 *
 * Design Pattern: Strategy / Provider Pattern
 * This allows the Input Binding Manager to treat different sources (e.g., Pawn, Vehicle, PlayerState) uniformly.
 *
 * Responsibilities:
 * - Expose a list of InputConfigs (Data Assets).
 * - Identify the lifecycle type of the source (Pawn-dependent vs Controller-persistent).
 */
class GAMECOREFRAMEWORK_API IGCFInputConfigProvider
{
	GENERATED_BODY()

public:
	/**
	 * Returns the source category of the input.
	 * Used to determine the lifecycle of the bindings (e.g., should they persist across death?).
	 */
	virtual EGCFInputSourceType GetInputSourceType() const = 0;

	/** Retrieves the list of currently active input configurations. */
	virtual TArray<const UGCFInputConfig*> GetInputConfigList() const = 0;

	/** Dynamically adds a new input config (e.g., when equipping a weapon). */
	virtual void AddInputConfig(const UGCFInputConfig* InputConfig) = 0;

	/** Removes an existing input config. */
	virtual void RemoveInputConfig(const UGCFInputConfig* Config) = 0;
};