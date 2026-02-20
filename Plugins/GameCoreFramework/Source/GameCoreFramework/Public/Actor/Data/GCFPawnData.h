// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GCFPawnData.generated.h"

#define UE_API GAMECOREFRAMEWORK_API 

class APawn;
class UGCFAbilitySet;
class UGCFCameraMode;
class UGCFInputConfig;
class UGCFAbilityTagRelationshipMapping;
class UGCFMovementConfig;
class UInputMappingContext;

/**
 * FGCFInputMappingContextInfo
 *
 * Struct used to define an Input Mapping Context and its associated priority.
 */
USTRUCT(BlueprintType)
struct FGCFInputMappingContextInfo
{
	GENERATED_BODY()

	// The Input Mapping Context to apply.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<const UInputMappingContext> InputMapping;

	// The priority to apply this context with. Higher values take precedence when keys conflict.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	int32 Priority = 0;
};


/**
 * UGCFPawnData
 *
 *	Non-mutable data asset that contains properties used to define a pawn.
 */
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = " Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class UGCFPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UE_API UGCFPawnData(const FObjectInitializer& ObjectInitializer);

public:

	// Class to instantiate for this pawn (should usually derive from APawn or ACharacter).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCF|Pawn")
	TSubclassOf<APawn> PawnClass;

	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCF|Abilities")
	TArray<TObjectPtr<UGCFAbilitySet>> AbilitySets;

	// What mapping of ability tags to use for actions taking by this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCF|Abilities")
	TObjectPtr<UGCFAbilityTagRelationshipMapping> TagRelationshipMapping;

	// Input mapping contexts to be applied by the player controller upon possessing this pawn.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCF|Input")
	TArray<FGCFInputMappingContextInfo> DefaultMappingContexts;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCF|Input")
	TObjectPtr<UGCFInputConfig> InputConfig;

	// Default camera mode used by player controlled pawns.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCF|Camera")
	TSubclassOf<UGCFCameraMode> DefaultCameraMode;

	// Movement parameter for pawn's movement component.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UGCFMovementConfig> MovementConfig;
};

#undef UE_API
