# Architecture Overview

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

### 4. Performance Maximization via Tickless Design
Fundamentally built on an **Event-Driven Model**, this framework completely eliminates reliance on per-frame `Tick` processing by enforcing `PrimaryComponentTick.bCanEverTick = false`.
By deeply integrating with Gameplay Messages, Component Extension Events, and multicast delegates, unnecessary state polling has been reduced. In real-world demo environment measurements using `stat unit`, we succeeded in keeping the **Game Thread processing time down to 3–4 ms**.
This maximizes scalability and ensures ample CPU headroom for complex gameplay logic and physics simulations.

---

## ✨ Technical Highlights of This Framework

### Dual ASC Architecture and Tag-Based Routing
In standard Gameplay Ability System (GAS) design, it is customary to place the Ability System Component (ASC) on *either* the PlayerState or the Pawn. However, to completely separate persistent abilities from swappable abilities, this framework adopts a **"Dual ASC Architecture," equipping both the PlayerState (Soul) and the Pawn (Body) with an ASC**.

The greatest technical challenge in this unique setup is the input routing problem: "When a player presses a button, to which ASC (Soul or Body) should the input be dispatched?"
To eliminate tight coupling where inputs directly reference specific Ability classes or ASCs, this framework introduces the `InputBridge` and `AbilityRouter`.

The `UGCFAbilityInputRouterComponent` automatically dispatches inputs based on the hierarchy of the tag passed during input, following these rules:
- 🟢 When an `Ability.Player.*` tag is received:
    - Routed to the **PlayerState's ASC** (Handled as a persistent ability of the soul, e.g., interaction).

- 🟠 When an `Ability.Pawn.*` tag is received:
    - Routed to the **Pawn's ASC** (Handled as an ability dependent on the current body, e.g., jumping or shooting).

As a result, the input side (Controller or InputComponent) does not need to know "whose ASC it is." It becomes possible to **dynamically switch the execution target simply by throwing a "Tag."**

---

## 🎯 Non-Goals and Target Scope

### Design Philosophy: Robustness over Speed
Rather than prototyping speed, this framework clearly prioritizes **Scalability** and **Deterministic Behavior**.
Adding a single button requires multiple steps (creating a DataAsset, assigning tags, etc.). However, this enforced structure ensures that "adding the 100th ability" is done with the same safety and order as "adding the 1st," preventing the project from devolving into spaghetti code.

**[Suitable Projects]**
- Large-scale multiplayer games or live-service titles.
- Developers looking to flexibly change Pawn compositions based on gameplay.
- Those seeking a robust design premised on multiplayer and Possession.

**[Unsuitable Projects]**
- Quick prototypes where "it just needs to work" (initial setup carries a learning curve).
- Simple games intended for a single Pawn and a single perspective.
- Complete replacements of official Unreal Engine implementations.

---

## 🚀 Future Outlook and Roadmap (Future Integrations)

In its initial phase, this framework prioritized proving the "robustness of the core foundation" (separation of concerns, asynchronous lifecycles, and established routing). Therefore, it was built as a **Minimum Viable Product (MVP), intentionally stripping away "cosmetic decorations" and "features dependent on specific game experiences" that do not directly affect the logic (Model) behavior**.

Now that the robust routing foundation and lifecycle management are complete, the next phase involves integrating the following features as **loosely coupled modules (GameFeatures or independent components)** on top of this skeleton:

**[Upcoming Modules for Implementation]**
- **User Facing Experience & UI Routing:** Full integration of `CommonUI` and `CommonGame` to build a seamless transition from the title screen to in-game, alongside a foundation for dynamic asset loading per Experience (leveraging PrimaryAssetLabels).

- **Cosmetic & Feedback System:** Utilizing features like `GameplayCue` from the Gameplay Ability System to integrate mechanisms that dynamically apply visual and auditory feedback (e.g., character animations, VFX, sounds) without polluting the logic.

- **Equipment & Inventory System:** A system to manage the equipped state of weapons and items, and dynamically grant abilities.

---

*Note: This project is currently in a state of evolution, and the current design is not the final answer. The design and form of separation of responsibilities are expected to evolve further through future implementation and validation.*