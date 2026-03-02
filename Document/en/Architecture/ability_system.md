# GAS Integration and Ability Routing (Dual ASC & Router Pattern)

🌍 *Read this in other languages: [English](../../en/Architecture/ability_system.md) | [日本語 (Japanese)](../../ja/Architecture/ability_system.md)* *(Note: The English documentation is AI-translated from the original Japanese).*

## Overview
This system is a core module designed to adapt Unreal Engine's Gameplay Ability System (GAS) for large-scale multiplayer games and complex possession requirements.

Its most prominent feature is the adoption of a **"Dual ASC Architecture," where both the PlayerState (Soul) and the Pawn (Body) are equipped with an Ability System Component (ASC).** By physically dividing the domains between "abilities the player should persistently retain (e.g., Interact, global cooldowns)" and "abilities completely dependent on the current vessel (e.g., magic attacks, driving a vehicle)," this approach achieves a design that **structurally minimizes the risk of bugs related to ability detachment failures and state loss**, which frequently plague complex possession mechanics.

Furthermore, by implementing the **"Ability Router Pattern,"** which automatically dispatches player inputs to the respective ASC based on tag prefixes, dynamic and safe context switching is realized.

---

## 🛑 Problems Solved

Typical GAS implementation tutorials instruct developers to place the ASC on *either* the Pawn or the PlayerState. However, blindly following this theory in production leads to the following issues:

1. **Loss of Persistence (Pawn-Only ASC):** If the ASC exists only on the Body, the moment the character dies and the Pawn is destroyed, all active buffs (cooldowns, persistent effects) vanish simultaneously.

2. **Ability Pollution (PlayerState-Only ASC):** If the ASC exists only on the Soul, boarding a vehicle or transforming into a monster causes "human abilities" and "vehicle abilities" to mix within the PlayerState. This drastically increases state management complexity and heightens the risk of architectural breakdown.

3. **Tight Coupling of Input and Abilities:** If inputs are hardcoded in C++ (e.g., `Jump Button = Trigger JumpAbility`), it becomes impossible to dynamically change this assignment (e.g., `Jump Button = Trigger FlyAbility`) when the player transfers to a different Body.

This system solves these challenges at the architectural level through the "complete separation of domains" and the "introduction of a routing layer."

---

## 📐 The Five Architecture Layers

<img width="8061" height="3999" alt="ability_system drawio" src="https://github.com/user-attachments/assets/a273a2ef-33df-42cc-bd43-3275f328efce" />

*▲ Click or download the image to enlarge and view the details of the class diagram.*

Based on the class diagram, this system's structure consists of the following five layers, each with clearly defined responsibilities:

### 1. Feature & Data Layer (Data-Driven Layer)
This layer defines abilities and inputs as data dynamically injected into the system, avoiding C++ hardcoding.

- **[`UGCFAbilitySet`][GCFAbilitySet] (DataAsset)**: Defines the mapping between the Ability Class (GA) granted to the character and the Input Tag used to trigger it.
- **[`UGCFInputConfig`][GCFInputConfig] (DataAsset)**: Defines the mapping between the player's physical operations (InputAction) and the aforementioned Input Tags.
- **`GameFeatureAction`**: Uses GameFeature functionality to dynamically inject the above assets (ability and input settings for the Soul/Body) without polluting existing code.

### 2. Input Binding Layer (Input Reception)
This layer safely receives physical inputs (e.g., button presses) from the player.

- **[`UGCFInputBindingManagerComponent`][GCFInputBindingManagerComponent]** / **[`UGCFInputComponent`][GCFInputComponent]**: Attached to the PlayerController. They execute Enhanced Input binding safely only when asynchronous loading and possession states are fully ready.

### 3. Ability Routing Layer (The Core of the System)
This layer receives inputs and dispatches them to the appropriate ASC.

- **[`UGCFPlayerInputBridgeComponent`][GCFPlayerInputBridgeComponent]** / **[`UGCFPawnInputBridgeComponent`][GCFPawnInputBridgeComponent]**: Requests bindings from the Input Layer and, when an input occurs, bridges the gap by sending *only* the "GameplayTag" to the Router.
- **[`UGCFAbilityInputRouterComponent`][GCFAbilityInputRouterComponent]**: Parses the prefix of the incoming GameplayTag and automatically routes it according to the following rules:
  - Tags matching **`InputTag.Ability.Player.*`** → Sent to the **PlayerState's ASC**.
  - Tags matching **`InputTag.Ability.Pawn.*`** → Sent to the **Pawn's ASC**.

### 4. Player Domain Layer (The Realm of the Soul)
This layer manages persistent data and abilities.

- **[`UGCFPlayerState`][GCFPlayerState]**: Functions as the player's "Soul" and holds its own unique **[`UGCFAbilitySystemComponent`][GCFAbilitySystemComponent]**.
- **Role:** Manages abilities that must be maintained even when swapping Pawns (Bodies), such as the player's level-up skills, inventory item effects, and global cooldowns.

### 5. Pawn Domain Layer (The Realm of the Body)
This layer manages the temporary vessel and its physical abilities.

- **[`UGCFCharacterWithAbilities`][GCFCharacterWithAbilities]**: The "Body" operated by the player, which also holds its own unique **[`UGCFAbilitySystemComponent`][GCFAbilitySystemComponent]**.
- **Role:** Manages innate abilities completely dependent on that specific vessel, such as "bipedal jumping," "shooting a gun," or "stepping on a car's accelerator."

---

## ⚙️ Core Mechanism: Tag-Based Automatic Dispatch Flow

The sequence from when a player presses a button to when an ability is triggered is as follows:

1. **Input Reception:** When the player presses a button, the event reaches either the [`UGCFPlayerInputBridgeComponent`][GCFPlayerInputBridgeComponent] or [`UGCFPawnInputBridgeComponent`][GCFPawnInputBridgeComponent] via the [`UGCFInputBindingManagerComponent`][GCFInputBindingManagerComponent].

2. **Tag Transmission (Bridge):** The Bridge component does not transmit "which button was pressed." Instead, it sends *only* the **"GameplayTag"** defined in the InputConfig (e.g., `InputTag.Ability.Pawn.Jump`) to the Router.

3. **Dynamic Routing (Router):** The [`UGCFAbilityInputRouterComponent`][GCFAbilityInputRouterComponent] parses the tag's prefix. During this step, the controller does not need to know "whether I am currently possessing a human or driving a vehicle."

4. **ASC Notification & Execution:** The Router invokes `AbilityInputTagPressed()` on the target ASC (PlayerState or Pawn), executing the corresponding ability.

---

## 🎯 Benefits of This Design

- **Domain Separation for Bug-Free Complex Possession:** By separating and placing ASCs on both the "PlayerState (Soul)" and the "Pawn (Body)", this design strongly suppresses bugs caused by improper manual management of abilities and buffs.
  * **Soul's ASC (PlayerState):** Manages persistent states like "Interact" or "Global Cooldowns" that must be maintained across vessel swaps. Prevents bugs where cooldown timers or buffs unexpectedly disappear (reset) when a Pawn is destroyed.
  * **Body's ASC (Pawn):** Manages vessel-specific abilities like "Magic Attacks" or "Vehicle Accelerator." The moment the player possesses a different Pawn, the old ASC and its abilities are "left behind" (removed from processing targets). This is designed to suppress issues where abilities fail to detach properly (e.g., preventing a player from casting magic while driving a car).

- **High-Level Decoupling:** The input side (Controller) does not need to be aware of the ability's implementation, and the ability side does not need to be aware of which button it is assigned to. Everything communicates solely through the common language of "Tags."

- **Safe Possession Implementation:** When boarding a vehicle (Possession), the link to the old Pawn's ASC is severed, and the routing destination automatically switches to the new Pawn's (Vehicle's) ASC. Communication to the Soul's ASC (PlayerState) is maintained as is, ensuring the system does not break.

- **Planner-Driven Development:** When adding new skills, programmers no longer need to write input binding code in C++. Planners simply register the tag in the DataAsset ([`UGCFInputConfig`][GCFInputConfig]), and the system automatically wires it to the appropriate ASC.


[GCFInputConfig]:                   ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputConfig.h
[GCFInputComponent]:                ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputComponent.h
[GCFInputBindingManagerComponent]:  ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputBindingManagerComponent.h
[GCFPlayerInputBridgeComponent]:    ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFPlayerInputBridgeComponent.h
[GCFPawnInputBridgeComponent]:      ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFPawnInputBridgeComponent.h
[GCFAbilityInputRouterComponent]:   ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFAbilityInputRouterComponent.h

[GCFAbilitySet]:                    ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/AbilitySystem/GCFAbilitySet.h
[GCFAbilitySystemComponent]:        ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/AbilitySystem/GCFAbilitySystemComponent.h

[GCFPlayerState]:                   ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Player/GCFPlayerState.h
[GCFCharacterWithAbilities]:        ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Character/GCFCharacterWithAbilities.h