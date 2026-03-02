# Actor Control System (Interface-Driven Opt-In Control)

🌍 *Read this in other languages: [English](../../en/Architecture/control_system.md) | [日本語 (Japanese)](../../ja/Architecture/control_system.md)* *(Note: The English documentation is AI-translated from the original Japanese).*

## Overview
This system provides an architecture designed to clearly separate and abstract the player's "movement intent (input)" from the body's "physical behavior (walking, driving, etc.)."

It prevents the hardcoding of movement logic into `ACharacter`—a frequent issue in standard Unreal Engine development—and establishes a loosely coupled movement control foundation utilizing Interfaces and a "Push ➔ Cache ➔ Pull" bucket relay mechanism.

---

## 🛑 Problems Solved

Writing movement logic directly within Pawn or Character classes leads to the following issues in production:

1. **Cast Proliferation Due to Base Class Dependency:** "Bipedal humans" and "four-wheeled vehicles" have different base classes (`ACharacter` and `AWheeledVehiclePawn`). Hardcoding movement logic causes the complexity of branching (Casting) to increase significantly when trying to operate them with the same controller, leading to bloated code.

2. **Direct References to Physics Components:** Tight coupling to specific physics APIs, such as the standard `CharacterMovementComponent` or the new `Mover` feature, makes it difficult to apply data (like movement speed) through a common flow and hinders component replacement.

3. **Tight Coupling of Movement Vectors and Cameras:** If the movement direction is too rigidly tied to the camera's rotation, it becomes difficult to implement flexible presentations or viewpoint controls, such as "moving the camera independently while the controlled character remains fixed."

This system solves these challenges through "abstraction via interfaces" and "strict domain separation."

---

## 📐 The Four Architecture Layers

<img width="11889" height="7089" alt="control_system drawio" src="https://github.com/user-attachments/assets/05639ceb-493b-43ff-9841-2513ced4837c" />

*▲ Click or download the image to enlarge and view the details of the class diagram.*

The structure of this system is broadly divided into the following four layers. Its defining characteristic is the clear separation between the "Soul (Controller)" and the "Body (Pawn)."

### 1. Input & Controller Layer (Soul / Universal Intent Layer)
This layer receives input from the player and calculates a universal intent ("how I want to move") that does not depend on the physical body.

- **[`UGCFLocomotionDirectionComponent`][GCFLocomotionDirectionComponent]**: Attached to the PlayerController (Soul). It processes universal movement vectors (Direction) from analog sticks and pushes the intent without caring about the structure of the target body.

- **[`UGCFCameraControlComponent`][GCFCameraControlComponent]**: Handles viewpoint operations and controls the camera's rotation (ControlRotation) completely independently of the movement logic.

### 2. Pawn & Implementation Layer (Body / Specific Action & Intent Cache Layer)
This layer caches universal commands from the controller and processes action inputs that depend on specific bodies, such as "Jump" or "Crouch."
- **GCF Pawn 3-Tier Architecture**: Strictly divides Pawn classes based on physical requirements.
  - [`AGCFPawn`][GCFPawn]: The purest base class, solely responsible for caching intents.
  - [`AGCFAvatarPawn`][GCFAvatarPawn]: Possesses a skeletal mesh; handles shape-independent universal actions like "Jump."
  - [`AGCFHumanoid`][GCFHumanoid]: Possesses a capsule collision; handles specialized actions highly dependent on capsules, like "Crouch."

- **[`UGCFLocomotionActionComponent`][GCFLocomotionActionComponent]**: Attached to the Pawn (Body) to push intents for specific actions (Jump/Crouch, etc.).

- **[`IGCFLocomotionInputHandler`][GCFLocomotionInputHandler]**: An interface for "Pushing" intents from the input component to the Pawn.

- **[`IGCFLocomotionInputProvider`][GCFLocomotionInputProvider]**: A read-only interface used by the Mover's producer to safely "Pull" the intents cached in the Pawn.

### 3. Data & Simulation Layer (Data-Driven / Physics Simulation Layer)
This layer manages actual physical movement and parameters (speed, acceleration, etc.).
- **[`UGCFLocomotionInputProducer`][GCFLocomotionInputProducer]**: In UE5's Mover plugin, this component pulls intents from the Pawn via interfaces, translates them, and transmits them to the physics engine.
  - *Note: If you need to pass specialized input structures to Mover (e.g., [`FGCFHumanoidInputs`][GCFHumanoidInputs] for crouching), you can safely inject this by creating a derived [`UGCFHumanoidInputProducer`][GCFHumanoidInputProducer], reusing the core logic.*

- **[`UGCFMovementConfig`][GCFMovementConfig] (DataAsset)**: Holds movement parameters that can be adjusted by planners.

- **[`IGCFMovementConfigReceiver`][GCFMovementConfigReceiver]**: An interface that allows parameters to be applied via a common flow, regardless of whether the internal physics component is Mover or the legacy CMC.

### 4. SubSystem & Camera Layer (Camera / State Monitoring Layer)
This layer separates camera behavior from movement logic and controls it via message broadcasting.
- **`UGameplayMessageSubsystem`**: Broadcasts controller calculation results or camera policy changes without direct event binding.

- **[`GCFCameraMode`][GCFCameraMode]**: Abstracts camera policies (Orbit, ThirdPerson, etc.) and flexibly switches camera behavior based on the movement state.

---

## ⚙️ Core Mechanism: The Intent "Bucket Relay" (Push ➔ Cache ➔ Pull)

The sequence from when a player tilts a thumbstick to when the character moves is handled in a 3-step "bucket relay" optimized for the Mover plugin.

1. **Intent Transmission (Push):** When the [`UGCFLocomotionDirectionComponent`][GCFLocomotionDirectionComponent] or [`UGCFLocomotionActionComponent`][GCFLocomotionActionComponent] detects input, it dispatches (Pushes) the intent ("I want to move this way", "I want to jump") via the **[`IGCFLocomotionInputHandler`][GCFLocomotionInputHandler]** without casting to a specific Pawn class.

2. **Relay and Retention (Cache & Opt-In):** The Pawn receiving the command safely ignores actions it cannot execute (e.g., a car crouching) thanks to UHT's (Unreal Header Tool) default interface implementation. It overrides (Opts-in) only the features it supports and saves (caches) the state in variables in preparation for the Mover's requests.

3. **Intent Extraction and Execution (Pull):** Every frame, the Mover's **[`UGCFLocomotionInputProducer`][GCFLocomotionInputProducer]** (or its derived classes) actively extracts (Pulls) the cached movement vectors, orientation, and action intents from the Pawn via the **[`IGCFLocomotionInputProvider`][GCFLocomotionInputProvider]**, applying them to the actual physics simulation.

---

## 🎯 Benefits of This Design

- **High-Level Plug & Play** When adding a new "flying vehicle" or a "monster with entirely new walking algorithms," the system can be expanded without modifying existing core logic. Simply implementing the interface (opting-in) on the new Pawn makes it instantly controllable.

- **Zero-Boilerplate Design Leveraging UHT** By utilizing the default behavior of interfaces, this design minimizes redundant code (boilerplate), such as forcing a "non-jumping car" to implement numerous empty jump functions.

- **Core Logic Reuse and Safe Scalability** Common operations like movement and jumping are handled by a single base Producer. Only when unique processing (like crouching) is required do you inherit and expand the Producer. This scales safely across the entire project without creating tight coupling.

- **Team Development Scalability** As long as programmers adhere to the "Interface Standards (Push/Pull)," they are free to implement the internal logic of individual Pawns however they see fit. This prevents conflicts and cascading bugs during parallel development by multiple people.

- **Seamless Integration of Event-Driven and Tick-Based Simulations** The Mover plugin requires Tick-based input prediction, whereas this framework is event-driven. To absorb this paradigm difference, we built a "Cache & Pull Mechanism" using the Pawn as a relay point. This maintains the lightness of event-driven architecture while fully utilizing Mover's powerful network synchronization and rollback capabilities.


[GCFCharacter]:                          ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Character/GCFCharacter.h
[GCFPawn]:                               ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Pawn/GCFPawn.h
[GCFAvatarPawn]:                         ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Avatar/GCFAvatarPawn.h
[GCFHumanoid]:                           ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Humanoid/GCFHumanoid.h
[GCFWheeledVehiclePawn]:                 ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Vehicle/GCFWheeledVehiclePawn.h

[GCFLocomotionDirectionComponent]:       ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Locomotion/GCFLocomotionDirectionComponent.h
[GCFLocomotionActionComponent]:          ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Locomotion/GCFLocomotionActionComponent.h
[GCFLocomotionInputHandler]:             ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Locomotion/GCFLocomotionInputHandler.h
[GCFLocomotionInputProvider]:            ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Locomotion/GCFLocomotionInputProvider.h
[GCFMoverComponent]:                     ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Mover/GCFMoverComponent.h
[GCFLocomotionInputProducer]:            ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Mover/Producer/GCFLocomotionInputProducer.h
[GCFHumanoidInputProducer]:              ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Mover/Producer/GCFHumanoidInputProducer.h
[GCFHumanoidInputs]:                     ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Mover/Input/GCFHumanoidInputs.h
[GCFCharacterMovementComponent]:         ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/GCFCharacterMovementComponent.h
[GCFMovementConfig]:                     ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/GCFMovementConfig.h
[GCFMovementConfigReceiver]:             ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/GCFMovementConfigReceiver.h

[GCFCameraMode]:                         ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Camera/Mode/GCFCameraMode.h
[GCFCameraControlComponent]:             ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Camera/GCFCameraControlComponent.h

[GCFInputComponent]:                     ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputComponent.h