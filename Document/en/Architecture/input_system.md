# Input System (InputBridge & Manager Pattern)

## Overview
This system is an input routing foundation built upon Unreal Engine 5's "Enhanced Input System." It is specifically designed to resolve asynchronous issues (initialization race conditions) unique to multiplayer games and possession mechanics.

To ensure that input-processing components do not directly depend on specific input configurations (`InputConfig`) or target actors (Pawns, etc.), the system is completely decoupled using the **Bridge Pattern** and **Manager (Mediator) Pattern**.

---

## 🛑 Problems Solved

Traditional input binding implementations frequently encounter the following issues:

1. **Reliance on Fleeting Events (Race Conditions)**  
Relying on the uncertain timing of "the moment of possession" often leads to attempts to bind inputs before the Pawn's data has finished loading, resulting in frequent crashes due to null references.

2. **Hardcoding Input Logic in Pawns**  
Hardcoding input handling directly within `SetupPlayerInputComponent` makes it extremely difficult to dynamically attach, detach, or switch inputs when transferring to a vehicle or another body.

3. **Mixing "Soul" and "Body" Inputs**  
Writing inputs for opening system menus (the Soul) and jumping (the Body) in the exact same place leads to bloated, monolithic classes with ambiguous responsibilities.

This system resolves these issues at the architectural level by dividing the input pipeline into "4 Layers."

---

## 📐 The 4 Layers of the Architecture

[![Input System Architecture](../Assets/Images/input_system.drawio.png)](../Assets/Images/input_system.drawio.png)
*▲ Click or download the image to view the class diagram in detail.*

The class structure of this system is broadly categorized into four layers:

### 1. Feature & Data Layer (Data-Driven Layer)
This layer defines inputs as data assets rather than hardcoding them in C++.

- **[`UGCFPawnData`][GCFPawnData]** / **[`UGCFInputConfig`][GCFInputConfig]**: Holds the Input Mapping Contexts (IMC) and input action definitions that a specific Pawn should possess.
- **`GameFeatureAction`**: Dynamically injects input definitions using configuration files tied to the player via GameFeatures, without modifying the existing codebase.

### 2. Lifecycle & Framework Layer (State & Lifecycle Management Layer)
This layer absorbs discrepancies caused by possession and asynchronous loading, guaranteeing safe execution timing.

- **`UGameFrameworkComponentManager` (GFCM):** Synchronizes and monitors the Feature States across the entire system.
- **[`UGCFPawnReadyStateComponent`][GCFPawnReadyStateComponent]** / **[`UGCFPlayerReadyStateComponent`][GCFPlayerReadyStateComponent]**: Individually monitors the initialization states of the Body (Pawn) and the Soul (PlayerState).
- **[`UGCFInputContextComponent`][GCFInputContextComponent]**: Monitors each ReadyState via the [`FGCFContextBinder`][GCFContextBinder] and notifies the Manager when a safe state for input binding (the Context) is fully prepared.

### 3. Core Manager & Routing Layer (Mediator & Bridge Layer)
The core of the system that connects binding requests to actual input configurations.

- **[`UGCFInputBindingManagerComponent`][GCFInputBindingManagerComponent]**: The central hub that receives all input binding requests. If states are incomplete, it queues the requests via `ProcessPendingBindings()` and safely executes the bindings the exact moment it receives the green light from the [`UGCFInputContextComponent`][GCFInputContextComponent].
- **[`UGCFPlayerInputBridgeComponent`][GCFPlayerInputBridgeComponent]** / **[`UGCFPawnInputBridgeComponent`][GCFPawnInputBridgeComponent]**: Implements the [`IUGCFInputConfigProvider`][GCFInputConfigProvider] interface, abstracting whether the configuration is for the "Soul" or the "Body," and provides it to the Manager.

### 4. Consumer Layer
This layer drives game logic using the actual inputs.

- **[`UGCFMovementControlComponent`][GCFMovementControlComponent]** / **[`UGCFCameraControlComponent`][GCFCameraControlComponent]**: These components do not need to know "what kind of Pawn" they are controlling. They simply send a Request to the [`UGCFInputBindingManagerComponent`][GCFInputBindingManagerComponent], asking it to "bind the movement inputs."

---

## ⚙️ Core Mechanism: Safe Asynchronous Binding Flow

The flow from requesting an input binding to its safe completion in this system operates as follows:

1. **Request**  
   A consumer (e.g., [`UGCFMovementControlComponent`][GCFMovementControlComponent]) requests the [`UGCFInputBindingManagerComponent`][GCFInputBindingManagerComponent] to bind an input action.

2. **Pending**  
   At this stage, the Pawn's data might still be loading asynchronously. The Manager does not execute the request immediately; instead, it temporarily stores it in an **internal queue (PendingBindings)**.

3. **Context Ready**  
   Once data loading and possession are complete under the strict management of GFCM, the [`UGCFInputContextComponent`][GCFInputContextComponent] fires an event signaling that "the input context is ready (`OnContextChanged`)."

4. **Execute**  
   The Manager receives this notification and simultaneously executes the addition of Mapping Contexts to the `UEnhancedInputLocalPlayerSubsystem` and the actual delegate bindings to the [`UGCFInputComponent`][GCFInputComponent].

---

## 🎯 Benefits of This Design

- **Complete Crash Resistance**  
Even in asynchronous environments or under severe network lag, crashes caused by null pointer references are fundamentally prevented.
  
- **True Plug & Play**  
When adding new vehicles or characters, no C++ code changes are required. Simply assigning a new InputConfig inside the [`UGCFPawnData`][GCFPawnData] enables the system to route inputs automatically.
  
- **Dynamic Input Attachment/Detachment**  
When possession ends, the Manager calls `ClearBindingsOnContextChange()` to cleanly scrub the old body's inputs. This prevents input "ghosting" (bugs where inputs continue to process for an uncontrollable actor).


[GCFPawnData]: ../../Source/GameCoreFramework/Public/Actor/Data/GCFPawnData.h

[GCFContextBinder]: ../../Source/GameCoreFramework/Public/System/Binder/GCFContextBinder.h
[GCFPawnReadyStateComponent]: ../../Source/GameCoreFramework/Public/System/Lifecycle/GCFPawnReadyStateComponent.h
[GCFPlayerReadyStateComponent]: ../../Source/GameCoreFramework/Public/System/Lifecycle/GCFPlayerReadyStateComponent.h

[GCFInputContextComponent]: ../../Source/GameCoreFramework/Public/Input/GCFInputContextComponent.h
[GCFInputComponent]: ../../Source/GameCoreFramework/Public/Input/GCFInputComponent.h
[GCFInputConfig]: ../../Source/GameCoreFramework/Public/Input/GCFInputConfig.h
[GCFInputBindingManagerComponent]: ../../Source/GameCoreFramework/Public/Input/GCFInputBindingManagerComponent.h
[GCFPlayerInputBridgeComponent]: ../../Source/GameCoreFramework/Public/Input/GCFPlayerInputBridgeComponent.h
[GCFPawnInputBridgeComponent]: ../../Source/GameCoreFramework/Public/Input/GCFPawnInputBridgeComponent.h
[GCFInputConfigProvider]: ../../Source/GameCoreFramework/Public/Input/GCFInputConfigProvider.h

[GCFMovementControlComponent]: ../../Source/GameCoreFramework/Public/Movement/GCFMovementControlComponent.h
[GCFCameraControlComponent]: ../../Source/GameCoreFramework/Public/Camera/GCFCameraControlComponent.h