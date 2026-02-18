# GAS Integration and Ability Routing (Dual ASC & Router Pattern)

## Overview
This system is a core module designed to adapt Unreal Engine's Gameplay Ability System (GAS) for large-scale multiplayer games and complex possession requirements.

Its most prominent features are the adoption of the **"Dual ASC Architecture"**, where both the `PlayerState` (the "Soul") and the `Pawn` (the "Body") have their own Ability System Component (ASC), and the implementation of the **"Ability Router Pattern"**, which automatically dispatches player inputs to the appropriate ASC based on tag prefixes.

---

## 🛑 Problems Solved

Standard GAS implementation tutorials usually instruct you to place the ASC on *either* the Pawn or the PlayerState. However, blindly following this theory in a production environment leads to the following issues:

1. **Loss of Persistence (Pawn-Only ASC)**  
   If the ASC is only attached to the physical body, the moment the character dies and the Pawn is destroyed, all maintained buffs (e.g., cooldowns, persistent status effects) are completely wiped out.

2. **Pollution of Abilities (PlayerState-Only ASC)**  
   If the ASC is only attached to the soul, boarding a vehicle or transforming into a fundamentally different monster mixes "human bipedal abilities" and "vehicle driving abilities" inside the same PlayerState, causing state management to completely break down.

3. **Tight Coupling of Inputs and Abilities**  
   Hardcoding logic in C++ (e.g., "Jump Button = Trigger JumpAbility") makes it impossible to dynamically reassign inputs when the player transfers to a new body (e.g., changing the "Jump Button" to trigger a "Fly Ability" instead).

This system resolves these issues at the architectural level through "Complete Domain Separation" and the introduction of a "Routing Layer."

---

## 📐 The 5 Layers of the Architecture

<img width="8061" height="3999" alt="ability_system drawio" src="https://github.com/user-attachments/assets/a273a2ef-33df-42cc-bd43-3275f328efce" />

*▲ Click or download the image to view the class diagram in detail.*

Based on the class diagram, the structure of this system is composed of five distinct layers with clearly defined responsibilities:

### 1. Feature & Data Layer (Data-Driven Layer)
This layer holds the definitions of abilities and inputs as data, injecting them dynamically into the system.

- **[`UGCFInputConfig`][GCFInputConfig] (DataAsset)**: Defines which input action triggers which `GameplayTag`.
- **`GameFeatureAction`**: Utilizes GameFeature functionality to dynamically inject abilities (for both soul and body) and InputConfigs without polluting the base codebase.

### 2. Input Binding Layer (Reception Layer)
Safely handles physical inputs from the player (e.g., button presses).

- **[`UGCFInputBindingManagerComponent`][GCFInputBindingManagerComponent]** / **[`UGCFInputComponent`][GCFInputComponent]**: Attached to the Player Controller, these components safely execute Enhanced Input bindings only when asynchronous loading and possession states are fully ready.

### 3. Ability Routing Layer (The Core of the System)
Receives the input and dispatches it to the appropriate ASC.

- **[`UGCFPlayerInputBridgeComponent`][GCFPlayerInputBridgeComponent]** / **[`UGCFPawnInputBridgeComponent`][GCFPawnInputBridgeComponent]**: Requests bindings from the Input Layer. When an input occurs, they act as a bridge, sending *only* the `GameplayTag` to the Router.
- **[`UGCFAbilityInputRouterComponent`][GCFAbilityInputRouterComponent]**: Parses the prefix of the incoming `GameplayTag` and automatically routes it according to the following rules:
  - Tags starting with **`Ability.Player.*`** → Routed to the **PlayerState's ASC**.
  - Tags starting with **`Ability.Pawn.*`** → Routed to the **Pawn's ASC**.

### 4. Player Domain Layer (The Realm of the Soul)
Manages persistent data and abilities.

- **[`UGCFPlayerState`][GCFPlayerState]**: Acts as the player's "Soul" and holds its own dedicated **[`UGCFAbilitySystemComponent`][GCFAbilitySystemComponent]**.
- **Role:** Manages abilities that must be maintained even when the player switches bodies, such as level-up skills, inventory item effects, and global cooldowns.

### 5. Pawn Domain Layer (The Realm of the Body)
Manages the temporary vessel and its physical capabilities.

- **[`UGCFCharacterWithAbilities`][GCFCharacterWithAbilities]**: The physical "Body" controlled by the player, which also holds its own dedicated **[`UGCFAbilitySystemComponent`][GCFAbilitySystemComponent]**.
- **Role:** Manages innate abilities strictly dependent on that specific Pawn, such as "bipedal jumping," "firing a weapon," or "stepping on a car's accelerator."

---

## ⚙️ Core Mechanism: Tag-Based Automatic Routing Flow

The lifecycle from a player pressing a button to an ability executing flows as follows:

1. **Input Reception**  
   The player presses a button, and the event reaches either the [`UGCFPlayerInputBridgeComponent`][GCFPlayerInputBridgeComponent] or the [`UGCFPawnInputBridgeComponent`][GCFPawnInputBridgeComponent] via the [`UGCFInputBindingManagerComponent`][GCFInputBindingManagerComponent].

2. **Tag Transmission (Bridge)**  
   Instead of passing "which button was pressed," the Bridge component sends *only* the **`GameplayTag`** defined in the InputConfig (e.g., `Ability.Pawn.Jump`) to the Router.

3. **Dynamic Routing (Router)**  
   The [`UGCFAbilityInputRouterComponent`][GCFAbilityInputRouterComponent] parses the tag's prefix.
   At this point, the Controller does not need to know whether it is currently possessing a human or a car.

4. **ASC Notification & Execution**  
   The Router invokes `AbilityInputTagPressed()` on the target ASC (PlayerState or Pawn), executing the corresponding ability.

---

## 🎯 Benefits of This Design

- **Ultimate Decoupling**  
   The input side (Controller) does not need to know anything about the ability implementations, and the ability side does not need to know which physical buttons they are bound to. Everything communicates exclusively through the universal language of "Tags."

- **Safe Possession Implementation**  
   When a player enters a vehicle (Possession), the link to the old Pawn's ASC is severed, and the routing destination automatically switches to the new Pawn's (Vehicle's) ASC. Meanwhile, communication to the Soul's ASC (PlayerState) is perfectly maintained, preventing any systemic breakdowns.

- **Designer-Driven Development**  
   When adding new skills, programmers no longer need to write C++ code for input bindings. Game designers simply register tags in the DataAsset ([`UGCFInputConfig`][GCFInputConfig]), and the system automatically wires them to the appropriate ASC.


[GCFInputConfig]: ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputConfig.h

[GCFInputBindingManagerComponent]: ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputBindingManagerComponent.h
[GCFInputComponent]: ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputComponent.h

[GCFPlayerInputBridgeComponent]: ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFPlayerInputBridgeComponent.h
[GCFPawnInputBridgeComponent]: ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFPawnInputBridgeComponent.h
[GCFAbilityInputRouterComponent]: ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFAbilityInputRouterComponent.h

[GCFPlayerState]: ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Player/GCFPlayerState.h
[GCFAbilitySystemComponent]: ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/AbilitySystem/GCFAbilitySystemComponent.h

[GCFCharacterWithAbilities]: ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Character/GCFCharacterWithAbilities.h