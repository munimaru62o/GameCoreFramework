# Architecture Overview

🌍 *Read this in other languages: [English](../../en/Architecture/overview.md) | [日本語 (Japanese)](../../ja/Architecture/overview.md)* *(Note: The English documentation is AI-translated from the original Japanese).*

## Overview

This project, "GameCoreFramework (GCF)," takes the modern architecture of Unreal Engine 5 (such as Lyra Starter Game), deconstructs it, and redesigns it as a **universal game framework** independent of any specific game genre.

Instead of merely extracting convenient features, it is built on the premise of conditions where design is most likely to break down in production: multiplayer, possession, late-joiners, and dynamic feature injection.
It aims to provide an architectural answer to questions like "Why does it break?" and "Where should the boundaries of responsibility be drawn?"

---

## 🛑 Problems Solved and Objectives

In game development involving multiplayer and complex player behaviors, the following problems frequently occur:

- **Ambiguous Responsibilities:** Hesitation over where to place logic (Pawn, Controller, or PlayerState) leads to the creation of God Classes.

- **Initialization Race Conditions:** Input, Ability, and Camera systems depend on each other, causing crashes due to differences in load order.

- **State Desync:** Network latency and late-joiners cause discrepancies between visual representation and internal state.

- **Chain of Tight Coupling:** Direct coupling between inputs and abilities makes it difficult to swap out actors or playable characters.

The primary objective of this project is to create a structure that prevents these issues **through systemic architecture, rather than relying on operational care (the programmer's attentiveness)**.

### 💡 Benefits of Adoption

By adopting this framework, development teams will reap the following benefits:

- Eliminates confusion about "where to write what" when dealing with possession and late-joiners.

- Frees developers from the hassle of individually tweaking initialization order bugs and complex dependencies between systems.

- Ensures the core design remains intact even when expanding gameplay mechanics like "riding vehicles" or "transforming into other characters."

---

## 📐 Core Design Philosophy

### 1. Complete Separation of "Soul" (Player) and "Body" (Pawn)

This framework clearly separates the responsibilities of persistent data and temporary vessels.

- **Player (Soul / Persistent):**
  Defines stats and persistent abilities that are maintained even when switching bodies (Pawns) (primarily handled by the PlayerState).
  Uses "Possession" of a body as a trigger to activate the features inherent to that body.

- **Pawn (Body / Temporary):**
  A physical entity that only defines the innate abilities and input bindings that its "shell" should possess (e.g., "accelerate/brake" for a car, "jump/walk" for a human).

This separation prevents logic from breaking in games involving possession and allows for the natural implementation of gameplay involving body-swapping.

### 2. Hybridization of Event-Driven and State-Based Models
Initially, the design was purely event-driven (using notifications via Delegates). However, we faced limitations unique to asynchronous multiplayer, such as "late-joiners missing past events" and "crashes caused by initialization order when relying on fleeting events like `OnPossessed`."

Therefore, we transitioned to a **hybrid architecture that treats these as "States" rather than events**, utilizing the `InitState` of the GameFrameworkComponentManager (GFCM). By implementing strict gate control that "prevents progressing to the next state until data, possession, and logic are all present," we achieved a robust structure where "the current situation can be perfectly replicated at any time of joining, based on the state at that moment."

### 3. Gameplay Expansion via Data-Driven Design
We adopted a design where behaviors and abilities are not hardcoded in C++, but rather expanded through variations in DataAssets. Pawn and Ability configurations can be flexibly switched according to the gameplay.

### 4. Minimizing Baseline Overhead via Tickless Design
Fundamentally built on an **Event-Driven Model**, this framework strictly eliminates reliance on per-frame `Tick` processing by enforcing `PrimaryComponentTick.bCanEverTick = false` across its core components.
By deeply integrating with Gameplay Messages, Component Extension Events, and multicast delegates, unnecessary state polling has been eradicated.
This architectural approach effectively **reduces the framework's baseline CPU overhead to near zero**. By doing so, it maximizes overall scalability and ensures ample CPU headroom is preserved for what truly matters: complex gameplay logic, high-density actor management, and physics simulations.

---

## ✨ Technical Highlights of This Framework

### 1. Dual ASC Architecture and Tag-Based Routing

In standard Gameplay Ability System (GAS) design, it is customary to place the Ability System Component (ASC) on *either* the PlayerState or the Pawn. However, to completely separate persistent abilities from swappable abilities, this framework adopts a **"Dual ASC Architecture," equipping both the PlayerState (Soul) and the Pawn (Body) with an ASC**.

The greatest technical challenge in this unique setup is the input routing problem: "When a player presses a button, to which ASC (Soul or Body) should the input be dispatched?"
To eliminate tight coupling where inputs directly reference specific Ability classes or ASCs, this framework introduces the `InputBridge` and `AbilityRouter`.

The `UGCFAbilityInputRouterComponent` automatically dispatches inputs based on the hierarchy of the tag passed during input, following these rules:
- 🟢 When an `InputTag.Ability.Player.*` tag is received:
    - Routed to the **PlayerState's ASC** (Handled as a persistent ability of the soul, e.g., interaction).

- 🟠 When an `InputTag.Ability.Pawn.*` tag is received:
    - Routed to the **Pawn's ASC** (Handled as an ability dependent on the current body, e.g., jumping or shooting).

As a result, the input side (Controller or InputComponent) does not need to know "whose ASC it is." It becomes possible to **dynamically switch the execution target simply by throwing a "Tag."**

### 2. Complete Integration of the Next-Gen Movement System (Mover) & Loosely Coupled Architecture
To maximize the potential of the "Mover" plugin—Unreal Engine 5's next-generation movement system—we have built a clean, lightweight `APawn`-based movement foundation that does not rely on the legacy `ACharacter`.

- **The Intent Bucket Brigade (Push ➔ Cache ➔ Pull)**
  The transmission from the player's "operating intent" to the "physical behavior" is completely decoupled using interfaces. We have realized the ultimate loosely coupled architecture: the input side *Pushes* the intent without caring about the target's class, the Pawn interprets and holds (*Caches*) it according to its own physical body, and finally, the Mover's producer retrieves (*Pulls*) it. As a result, even if possession is transferred from a "human who can jump" to a "car that cannot jump", the controls switch seamlessly without any code breaking.

- **Maintaining Clock Sync in Hybrid Environments (Adapter Pattern)** In a game environment where the legacy `CharacterMovementComponent` (CMC) and the new `Mover` (Network Prediction Plugin) coexist, the simulation clock sleeps in isolation for each system. Because of this, when a player is controlling a CMC, network packets from another player's Mover (like a drone) are discarded as "future data," causing a freezing issue known as Extrapolation Starvation.
  In this system, we provide a **"lightweight dummy Mover with no physical collision"** as an Adapter to the `PlayerController`, which always communicates with the server. This establishes a robust infrastructure that keeps the network clock globally synchronized, regardless of which Pawn the player currently possesses.

---

## 🛠 Developer Experience (DX): Simple Extensibility Hiding Complex Internal Structures

Although this framework executes extremely complex asynchronous processing and routing internally, **it completely hides (encapsulates) this complexity from the programmers and planners (users) who actually implement the gameplay.**

When adding new features, users do not need to modify the existing core code at all. Safe expansion is possible with only the following extremely simple steps.

### Example 1: Binding New Abilities and Inputs by Planners (Data-Driven)
There is no need to write a single line of C++ code. By simply configuring two DataAssets that separate "ability definition" and "physical input," the system automatically determines the appropriate ASC (Soul or Body) and performs the routing.

#### 1. Assign an "Input Tag" to the Ability (`UGCFAbilitySet`)
Open the target ability set and configure the following elements:

- **Granted Gameplay Abilities:**
  - **Ability:** `GA_Jump` (The ability class to add)
  - **InputTag:** `InputTag.Ability.Pawn.Jump` (Target domain is automatically determined by the prefix)

#### 2. Bind Physical Operations (InputAction) to the Tag (`UGCFInputConfig`)
Open the input configuration and simply bind the standard Unreal InputAction to the tag defined above.

- **Ability Input Actions:**
  - **InputAction:** `IA_Jump` (Physical input such as Spacebar or gamepad button)
  - **GameplayTag:** `InputTag.Ability.Pawn.Jump`

### Example 2: Adding a "New Vehicle" by a Programmer (Opt-In Design via Interfaces)
For example, if you want to add a "Hoverboard" with a completely new physical behavior, you do not need to rewrite the input processing on the controller side.
Simply implement the `IGCFLocomotionInputHandler` interface in the new Pawn class, receive the transmitted universal "movement intent (vector)", and interpret it as your own unique propulsion.

```cpp
// AGCFHoverboardPawn.cpp
// By simply overriding the interface function, it receives the universal operating intent Pushed by the controller.
void AGCFHoverboardPawn::HandleMoveInput_Implementation(const FVector2D& InputValue, const FRotator& MovementRotation)
{
    // Cache the "intended movement direction" calculated by the controller,
    // and translate it into physical thruster processing specific to the hoverboard.
    CachedMoveInput = InputValue;
    CachedMoveRotation = MovementRotation;
    UpdateHoverThrusters();
}
```

In this way, simply by operating "inside the rules of the architecture," we provide a foundation where anyone can safely and rapidly mass-produce features.

---

## 🎯 Non-Goals and Target Scope

### Design Philosophy: Robustness over Speed

Rather than the initial speed of prototyping, this framework makes ensuring **"Mid-to-Long-Term Scalability"** and **"Deterministic Behavior"** its top design priorities.

To add a single button, it requires explicit procedures (rules) such as "creating a DataAsset" and "assigning a tag." Therefore, initial setup and learning require a certain cost. However, because of this enforced structure, **"adding the 100th ability" can be done with the exact same safety and order as "adding the 1st."**

When entering the mid-to-long-term specification changes or the mass-production phase with a team of dozens, the iteration speed enabled by this "unbreakable foundation" will ultimately vastly outperform traditional prototype-style development.

**[Projects Where It Delivers the Most Value]**
- Large-scale multiplayer games or long-term live-service titles planned for years of operation.
- Teams wanting to drastically reduce programmer QA (debugging) costs during content mass production.
- Games where "boarding vehicles" or "dynamic character changes (Possession)" are core mechanics.

**[Projects Where It Might Be Over-Engineering]**
- Mockup development intended to be thrown away in a few weeks without considering scalability.
- Small-scale games completed with only a single Pawn and single perspective, without any multiplayer or state transitions expected.

---

## 🚀 Future Outlook and Roadmap (Future Integrations)

In its initial phase, this framework prioritized proving the "robustness of the core foundation" (separation of concerns, asynchronous lifecycles, and established routing). Therefore, it was built as a **Minimum Viable Product (MVP), intentionally stripping away "cosmetic decorations" and "features dependent on specific game experiences" that do not directly affect the logic (Model) behavior**.

Now that the robust routing foundation and lifecycle management are complete, the next phase involves integrating the following features as **loosely coupled modules (GameFeatures or independent components)** on top of this skeleton:

**[Upcoming Modules for Implementation]**
- **User Facing Experience & UI Routing:** Full integration of `CommonUI` and `CommonGame` to build a seamless transition from the title screen to in-game, alongside a foundation for dynamic asset loading per Experience (leveraging PrimaryAssetLabels).

- **Cosmetic & Feedback System:** Utilizing features like `GameplayCue` from the Gameplay Ability System to integrate mechanisms that dynamically apply visual and auditory feedback (e.g., character animations, VFX, sounds) without polluting the logic.

- **Equipment & Inventory System:** A system to manage the equipped state of weapons and items, and dynamically grant abilities.

---

## 🖋️ Developer's Vision: A Philosophy on Architecture Design

To me, system architecture design transcends mere engineering; it is a form of art and self-expression. Releasing this "GameCoreFramework" as an open-source project is not simply about providing a convenient tool, but rather an exhibition of a "piece of work" that embodies my personal aesthetics.

Modern game development grows more complex by the day, yet I firmly believe that "if the internal structure is beautiful, the external behavior will inevitably be beautiful as well." Facing complex phenomena head-on, iterating through trial and error, and carving away the excess like a sculpture to form a single, polished crystal—I believe there is profound value in that very process of thought and exploration.

Even in a future where all development processes become increasingly automated and highly efficient, I believe that the skill and joy of grasping the true essence with our own hands and minds to build beautiful structures will never fade.

I hope that the design philosophies and ideas poured into this framework will serve as a source of inspiration for engineers around the world who share a love for "beautiful, highly maintainable systems."

---

*Note: This project is currently in a state of evolution, and the current design is not the final answer. The design and form of separation of responsibilities are expected to evolve further through future implementation and validation.*