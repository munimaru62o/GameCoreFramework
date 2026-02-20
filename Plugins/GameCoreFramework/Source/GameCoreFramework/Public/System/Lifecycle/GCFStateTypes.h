// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GCFStateTypes.generated.h"

/**
 * @brief Bitflags defining the initialization milestones for a Pawn ("Body").
 *
 * Used to track the asynchronous loading and initialization of various sub-systems.
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EGCFPawnReadyState : uint8
{
	None		= 0	UMETA(Hidden),
	PawnData	= 1 << 0, // The core PawnData asset has been loaded and applied.
	Ability		= 1 << 1, // The Ability System Component (ASC) is initialized.
	Possessed	= 1 << 2, // A valid Controller has possessed this Pawn.
	GamePlay	= 1 << 3, // All GameFramework extensions (Features) are ready.
};
ENUM_CLASS_FLAGS(EGCFPawnReadyState);


/**
 * @brief Bitflags defining the initialization milestones for a Player ("Soul").
 *
 * Tracks the readiness of the persistent player data and connection.
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EGCFPlayerReadyState : uint8
{
	None		= 0	UMETA(Hidden),
	Controller	= 1 << 0, // The PlayerController is fully initialized.
	PlayerState = 1 << 1, // The PlayerState has replicated core data (UniqueId, Team, etc.).
	Ability		= 1 << 2, // The PlayerState's ASC is initialized.
	Possession	= 1 << 3, // The player has successfully possessed a Pawn.
	GamePlay	= 1 << 4, // High-level gameplay features (HUD, InputConfig) are ready.
};
ENUM_CLASS_FLAGS(EGCFPlayerReadyState);


/**
 * Snapshot struct capturing the Pawn's state at a specific moment.
 * Passed to native delegates to ensure thread-safe(ish) access to the source object.
 */
struct FGCFPawnReadyStateSnapshot
{
	TWeakObjectPtr<APawn> Pawn;
	EGCFPawnReadyState State;

	FGCFPawnReadyStateSnapshot()
		: State(EGCFPawnReadyState::None)
	{}

	FGCFPawnReadyStateSnapshot(APawn* InPawn, EGCFPawnReadyState InState)
		: Pawn(InPawn), State(InState)
	{}
};


/**
 * Snapshot struct capturing the Player's state at a specific moment.
 */
struct FGCFPlayerReadyStateSnapshot
{
	TWeakObjectPtr<APlayerState> PlayerState;
	EGCFPlayerReadyState State;

	FGCFPlayerReadyStateSnapshot()
		: State(EGCFPlayerReadyState::None)
	{}

	FGCFPlayerReadyStateSnapshot(APlayerState* InPS, EGCFPlayerReadyState InState)
		: PlayerState(InPS), State(InState)
	{}
};

// ------------------------------------------------------------------------------------------------
// Delegates
// ------------------------------------------------------------------------------------------------

/**
 * Native Delegate for Pawn readiness changes.
 * @param Snapshot Contains the weak pointer to the Pawn and the new state bitmask.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FGCFOnPawnReadyStateChangedNative, const FGCFPawnReadyStateSnapshot& /* SnapShot */);

/**
 * Blueprint Delegate for Pawn readiness changes.
 * Only passes the state enum for simplicity in Blueprints.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGCFOnPawnReadyStateChangedBP, EGCFPawnReadyState, CurrentState);


/**
 * Native Delegate for Player readiness changes.
 * @param Snapshot Contains the weak pointer to the PlayerState and the new state bitmask.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FGCFOnPlayerReadyStateChangedNative, const FGCFPlayerReadyStateSnapshot& /* SnapShot */);

/**
 * Blueprint Delegate for Player readiness changes.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGCFOnPlayerReadyStateChangedBP, EGCFPlayerReadyState, CurrentState);