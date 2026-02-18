# Actor Control System (Interface-Driven Control System)

## Overview
This system is an architecture designed to completely decouple the player's "intention to move" (input) from the "physical behavior" (walking, driving, etc.) of the physical body. 

It prevents the hardcoding of movement logic into the `ACharacter` class—a common pitfall in standard Unreal Engine development—and provides a loosely coupled movement control foundation using interfaces and a component-oriented design.

---

## 🛑 Problems Solved

Hardcoding movement logic directly into Pawn or Character classes leads to the following issues in a production environment:

1. **Cast Proliferation Due to Base Class Dependency:**  
   "Bipedal humans" and "four-wheeled vehicles" have different base classes (`ACharacter` and `AWheeledVehiclePawn`). Hardcoding movement logic leads to an endless proliferation of conditional branches (Casts) to control them with the same controller, severely bloating the codebase.

2. **Direct References to Physics Components:**  
   Tight coupling to specific physical APIs, such as the standard `CharacterMovementComponent` or the new `Mover` plugin, makes it impossible to apply parameters (like movement speed) through a common flow, hindering the ability to easily swap components.

3. **Tight Coupling Between Movement Vectors and Cameras:**  
   If movement direction and camera orientation are too rigidly connected, flexible presentations and viewpoint controls—such as "moving the camera independently while keeping the controlled character stationary"—become impossible to implement cleanly.

This system solves these issues through "Abstraction via Interfaces" and "Domain Separation."

---

## 📐 The 4 Layers of the Architecture

[![Movement System Architecture](../Assets/Images/control_system.drawio.png)](../Assets/Images/control_system.drawio.png)
*▲ Click or download the image to view the class diagram in detail.*

Based on the class diagram, the structure of this system is broadly divided into the following four layers:

### 1. Input & Controller Layer (Operation & Universal Intent Layer)
Receives player input and calculates the universal intent of "how to move," independent of the physical body.
- **[`UGCFMovementControlComponent`][GCFMovementControlComponent]** / **[`UGCFCameraControlComponent`][GCFCameraControlComponent]**: Attached to the Player Controller (the "Soul"). They handle generic movement inputs (like analog stick events) and camera operations, containing absolutely no logic tied to specific bodies (like humans or cars).

### 2. Pawn & Implementation Layer (Body & Specific Execution Layer)
Receives universal commands from the controller and executes physical behaviors or unique actions specific to that "Body" (Pawn).
- **[`UGCFCharacterControlComponent`][GCFCharacterControlComponent]** / **[`UGCFVehicleControlComponent`][GCFVehicleControlComponent]**: Attached directly to the Pawn side (the "Body"). They handle specific input operations strictly dependent on that body type, such as jumping (for humans) or turning on headlights (for vehicles).
- **[`IGCFLocomotionHandler`][GCFLocomotionHandler]**: The core interface of this system. The Controller's **[`UGCFMovementControlComponent`][GCFMovementControlComponent]** transmits the movement vector to the body exclusively through this interface, without ever needing to check the target's class type.

### 3. Data Layer (Data-Driven Layer)
Manages movement parameters (speed, acceleration, etc.).
- **[`UGCFMovementConfig`][GCFMovementConfig] (DataAsset)**: Holds movement parameters that can be easily tuned by game designers.
- **[`IGCFMovementConfigReceiver`][GCFMovementConfigReceiver]**: An interface for receiving data. By having standard components like [`UGCFCharacterMovementComponent`][GCFCharacterMovementComponent] or the new [`UGCFMoverComponent`][GCFMoverComponent] implement this, parameters (Config) can be applied via a unified flow regardless of the underlying physics component.

### 4. SubSystem & Camera Layer (Camera & State Monitoring Layer)
Decouples camera behavior from movement logic and controls it via a message-based system.
- **`UGameplayMessageSubsystem`**: Broadcasts the controller's calculation results and camera policy changes without directly binding events.
- **[`GCFCameraMode`][GCFCameraMode]**: Abstracts camera policies (e.g., Orbit, Third-Person) and flexibly switches camera behavior based on the current movement state.

---

## ⚙️ Core Mechanism: Interface-Driven Movement Flow

The flow from the player tilting the stick to the character moving is as follows:

1. **Input Reception:**  
   The input action bound from the [`UGCFInputComponent`][GCFInputComponent] fires, calling `Input_Move()` on the [`UGCFMovementControlComponent`][GCFMovementControlComponent].

2. **Movement Vector Calculation:**  
   The controller calculates the actual movement vector the Pawn should take, based on the camera's current rotation.

3. **Command Transmission via Interface:**  
   The controller checks if the currently possessed Pawn implements the **[`IGCFLocomotionHandler`][GCFLocomotionHandler]**. If it does, it passes the calculated vector to `HandleMoveInput()`.

4. **Physical Execution per Body:**  
   If the command is received by an [`AGCFCharacter`][GCFCharacter], `AddMovementInput` is called. If received by an [`AGCFWheeledVehiclePawn`][GCFWheeledVehiclePawn], it is translated into throttle and steering processing.

---

## 🌐 Network Synchronization and Network Prediction Optimization

When deploying Unreal Engine 5's next-generation network synchronization framework, the Network Prediction Plugin (NPP), and the Mover plugin into a live environment, fatal sync discrepancies and prediction runaways caused by the engine's internal specifications can occur. This system resolves these critical issues through its unique architectural design.

### 1. Maintaining Clock Sync in a Hybrid Environment (Adapter Pattern)
In game environments where the legacy `CharacterMovementComponent` (CMC) and the new `Mover` coexist, NPP's simulation clock sleeps in isolation per system. Consequently, when a player is controlling a CMC, network packets from other players' Movers (e.g., drones) are discarded as "future data," causing freezing issues known as **Extrapolation Starvation**. 
This system establishes a robust infrastructure that keeps the NPP clock globally synchronized regardless of which Pawn the player is possessing. It achieves this by attaching a **"lightweight dummy Mover with no physical collision"** to the `PlayerController` (which is always communicating with the server) to act as an Adapter.

### 2. Safe Input Sanitization to Prevent Extrapolation Runaway
Mover's Simulated Proxies (other players' characters) predict future positions (Extrapolation) by reusing the last received input data when packet loss occurs. However, in low-friction states such as flying, this "stale input" causes the character to accelerate and move forward infinitely, resulting in severe rubber-banding when packets finally arrive. 
Without polluting NPP's core buffer, this system implements a hack that **forcibly clears only the directional vector of the "disposable input snapshot for prediction calculation"** generated every frame. This prevents fatal overshooting under laggy conditions while completely ensuring the safety of NPP's Rollback mechanism.

---

## 🎯 Benefits of This Design

- **Ultimate Plug & Play:**  
  When adding a new "flying vehicle" or a "monster with a completely new walking algorithm," you don't need to change a single line of code on the controller side (e.g., `Input_Move`). Simply implementing the [`IGCFLocomotionHandler`][GCFLocomotionHandler] on the new Pawn makes it instantly playable.

- **Coexistence of Old and New Physics Engines:**  
  Thanks to the [`IGCFMovementConfigReceiver`][GCFMovementConfigReceiver] interface, parameters can be streamed from the same DataAsset (MovementConfig) regardless of whether the Pawn uses the legacy CharacterMovement or the next-gen Mover plugin.

- **Team Development Scalability:**  
  As long as programmers adhere to the "Interface contract," they have complete freedom over the internal implementation of individual Pawns. This prevents conflicts and chained bugs during parallel development by multiple people.

- **Seamless Integration of Event-Driven and Tick-Based Simulations:**  
  The next-generation Mover plugin requires Tick-based input prediction, whereas this framework operates on an Event-Driven architecture. To absorb this paradigm shift, we built an "Input Cache Mechanism" where input results are temporarily synthesized and held as vectors, allowing the Mover's Producer ([`UGCFCachedInputProducer`][GCFCachedInputProducer]) to safely read them via an Interface. This fully supports Mover's powerful rollback mechanics while preserving the lightweight nature of event-driven execution.


[GCFMovementControlComponent]: ../../Source/GameCoreFramework/Public/Movement/GCFMovementControlComponent.h
[GCFCameraControlComponent]: ../../Source/GameCoreFramework/Public/Camera/GCFCameraControlComponent.h

[GCFCharacter]: ../../Source/GameCoreFramework/Public/Actor/Character/GCFCharacter.h
[GCFWheeledVehiclePawn]: ../../Source/GameCoreFramework/Public/Actor/Vehicle/GCFWheeledVehiclePawn.h
[GCFCharacterControlComponent]: ../../Source/GameCoreFramework/Public/Actor/Character/GCFCharacterControlComponent.h
[GCFVehicleControlComponent]: ../../Source/GameCoreFramework/Public/Actor/Vehicle/GCFVehicleControlComponent.h
[GCFLocomotionHandler]: ../../Source/GameCoreFramework/Public/Movement/GCFLocomotionHandler.h

[GCFMoverComponent]: ../../Source/GameCoreFramework/Public/Movement/GCFMoverComponent.h
[GCFCharacterMovementComponent]: ../../Source/GameCoreFramework/Public/Movement/GCFCharacterMovementComponent.h

[GCFMovementConfig]: ../../Source/GameCoreFramework/Public/Movement/GCFMovementConfig.h
[GCFMovementConfigReceiver]: ../../Source/GameCoreFramework/Public/Movement/GCFMovementConfigReceiver.h
[GCFLocomotionHandler]: ../../Source/GameCoreFramework/Public/Movement/GCFLocomotionHandler.h
[GCFCachedInputProducer]: ../../Source/GameCoreFramework/Public/Movement/Mover/GCFCachedInputProducer.h

[GCFCameraMode]: ../../Source/GameCoreFramework/Public/Camera/Mode/GCFCameraMode.h

[GCFInputComponent]: ../../Source/GameCoreFramework/Public/Input/GCFInputComponent.h