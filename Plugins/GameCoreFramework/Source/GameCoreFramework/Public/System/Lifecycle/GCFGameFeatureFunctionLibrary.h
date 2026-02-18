// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"

#include "GCFShared.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "GCFGameFeatureFunctionLibrary.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UGameFrameworkComponentManager;

/**
 * @brief Utility library for managing "Feature States" via the GameFrameworkComponentManager.
 *
 * [Design Philosophy]
 * - Features are persistent "States" (e.g., "PawnData Loaded", "Input Configured").
 * - Events are temporary triggers (handled elsewhere).
 *
 * This library provides a simplified API to transition features between "Initial" and "Ready" states,
 * allowing other components to wait for specific dependencies to be met.
 */
UCLASS(Abstract, MinimalAPI)
class  UGCFGameFeatureFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Transitions a feature to the "Ready" state.
	 * Call this when a specific dependency (e.g., PawnData) is fully loaded and applied.
	 *
	 * @param Implementer   The object responsible for this feature (usually 'this' component).
	 * @param Actor         The actor owning the feature (e.g., Pawn, PlayerState).
	 * @param FeatureName   The unique name of the feature (e.g., GCF::Names::Feature_Pawn_PawnData).
	 */
	UE_API static void ReadyFeature(UObject* Implementer, AActor* Actor, FName FeatureName);
	UE_API static void ReadyFeature(UGameFrameworkComponentManager* GFCM, UObject* Implementer, AActor* Actor, FName FeatureName);

	/**
	 * Resets or sets a feature to the "Initial" state.
	 * Call this when a feature is invalidated or starting setup.
	 */
	UE_API static void InitFeature(UObject* Implementer, AActor* Actor, FName FeatureName);
	UE_API static void InitFeature(UGameFrameworkComponentManager* GFCM, UObject* Implementer, AActor* Actor, FName FeatureName);
	
	/**
	 * Checks if a specific feature has reached the "Ready" state.
	 */
	static bool IsFeatureReady(AActor* Actor, FName FeatureName);
	static bool IsFeatureReady(UGameFrameworkComponentManager* GFCM, AActor* Actor, FName FeatureName);

	/**
	 * Checks if a feature is in a specific state tag.
	 */
	UE_API static bool IsFeatureTag(AActor* Actor, FName FeatureName, FGameplayTag Tag);
	UE_API static bool IsFeatureTag(UGameFrameworkComponentManager* GFCM, AActor* Actor, FName FeatureName, FGameplayTag Tag);
};

#undef UE_API