// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"

/**
 * Central registry for shared FName constants used throughout the project.
 *
 * [Purpose]
 * - Eliminates "Magic Strings" and prevents typos.
 * - Encapsulates the dynamic event names required by the GameFrameworkComponentManager (GFCM).
 * - Provides a single source of truth for Feature names and Event triggers.
 */
namespace GCF::Names
{
// ------------------------------------------------------------------------------------------------
// Game Feature / Extension System Events
// ------------------------------------------------------------------------------------------------

// Standard event sent by the engine when a new extension component is added.
inline const FName Event_Extension_Added			= TEXT("ExtensionAdded");

// Custom events for Pawn lifecycle & state transitions
inline const FName Event_Pawn_OnPossessed			= TEXT("GCF.Pawn.OnPossessed");
inline const FName Event_Pawn_OnUnPossessed			= TEXT("GCF.Pawn.OnUnPossessed");
inline const FName Event_Pawn_Ready_GamePlay		= TEXT("GCF.Pawn.Ready.GamePlay");
inline const FName Event_Pawn_ControllerAssigned	= TEXT("GCF.Pawn.ControllerAssigned");

// Custom events for Player (Controller/State) lifecycle
inline const FName Event_Player_Ready_Input			= TEXT("GCF.Player.Ready.Input");
inline const FName Event_Player_Ready_Controller	= TEXT("GCF.Player.Ready.Controller");
inline const FName Event_Player_UnReady_Controller	= TEXT("GCF.Player.UnReady.Controller");
inline const FName Event_Player_Ready_GamePlay		= TEXT("GCF.Player.Ready.GamePlay");

// ------------------------------------------------------------------------------------------------
// Feature Names (for InitState Registration)
// These correspond to specific "features" tracked by the GFCM (e.g., Feature_Pawn_Ability -> GAS)
// ------------------------------------------------------------------------------------------------

inline const FName Feature_Pawn_Extension			= TEXT("GCF.Feature.Pawn.Extension");
inline const FName Feature_Pawn_Possessed			= TEXT("GCF.Feature.Pawn.Possessed");
inline const FName Feature_Pawn_PawnData			= TEXT("GCF.Feature.Pawn.PawnData");
inline const FName Feature_Pawn_Ability				= TEXT("GCF.Feature.Pawn.Ability");
inline const FName Feature_Pawn_GamePlay			= TEXT("GCF.Feature.Pawn.GamePlay");

inline const FName Feature_Player_Extension			= TEXT("GCF.Feature.Player.Extension");
inline const FName Feature_Player_PlayerState		= TEXT("GCF.Feature.Player.PlayerState");
inline const FName Feature_Player_AbilitySystem		= TEXT("GCF.Feature.Player.AbilitySystem");
inline const FName Feature_Player_Controller		= TEXT("GCF.Feature.Player.Controller");
inline const FName Feature_Player_Possession		= TEXT("GCF.Feature.Player.Possession");
inline const FName Feature_Player_GamePlay			= TEXT("GCF.Feature.Player.GamePlay");
}