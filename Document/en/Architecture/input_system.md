# Input System (InputBridge & Manager Pattern)

🌍 *Read this in other languages: [English](../../en/Architecture/input_system.md) | [日本語 (Japanese)](../../ja/Architecture/input_system.md)* *(Note: The English documentation is AI-translated from the original Japanese).*

## Overview
This system is an input routing foundation built upon Unreal Engine 5's "Enhanced Input System." It is specifically designed to resolve asynchronous initialization issues (race conditions) that frequently occur in multiplayer and possession-based gameplay.

To prevent the components processing inputs from depending directly on specific input settings (InputConfig) or target actors (such as Pawns), it utilizes the **Bridge pattern** and **Manager (Mediator) pattern** to achieve a highly abstracted and loosely coupled architecture.

---

## 🛑 Problems Solved

Traditional input binding implementations often suffer from the following issues:

1. **Dependency on Fleeting Events (Race Conditions):** Relying on uncertain timings such as the exact "moment of possession" increases the risk of Null pointer reference crashes, as the system might attempt to bind inputs before the Pawn's data has finished loading.

2. **Hardcoded Input Processing in Pawns:** Writing logic directly within `SetupPlayerInputComponent` becomes a factor that hinders the flexible attachment and detachment of features when switching dynamically (e.g., when transitioning to a vehicle).

3. **Confusion Between "Soul" and "Body":** Inputs for opening system menus (Soul) and inputs for jumping (Body) end up being defined in the same place, leading to bloated classes.

This system solves these challenges at the architectural level by dividing the responsibilities into four distinct layers.

---

## 📐 The Four Architecture Layers

<img width="10509" height="5289" alt="input_system drawio" src="https://github.com/user-attachments/assets/6bb5c28e-f7a7-423f-add1-d2f1173a91a5" />

*▲ Click or download the image to enlarge and view the details of the class diagram.*

The class diagram of this system is broadly categorized into four layers:

### 1. Feature & Data Layer (Data-Driven Layer)
This layer defines inputs as DataAssets rather than hardcoding them in C++.

- **[`UGCFPawnData`][GCFPawnData]** / **[`UGCFInputConfig`][GCFInputConfig]**: Holds the "Input Mapping Context (IMC)" and Input Action definitions required for a specific Pawn.

- **`GameFeatureAction`:** Uses the GameFeature system to dynamically inject input definitions into players without modifying existing code.

### 2. Lifecycle & Framework Layer (State & Lifecycle Management Layer)
This layer absorbs the "desync" caused by possession and asynchronous loading, ensuring safe timing.

- **`UGameFrameworkComponentManager` (GFCM):** Synchronizes and monitors the Feature States across the entire system.

- **[`UGCFPawnReadyStateComponent`][GCFPawnReadyStateComponent]** / **[`UGCFPlayerReadyStateComponent`][GCFPlayerReadyStateComponent]**: Individually monitors the initialization states of the Body (Pawn) and the Soul (PlayerState).

- **[`UGCFInputContextComponent`][GCFInputContextComponent]**: Monitors various ReadyStates via [`FGCFContextBinder`][GCFContextBinder] and notifies the Manager when the "Input Context" (a safe state to perform input binding) is ready.

### 3. Core Manager & Routing Layer (Mediator & Bridge Layer)
The core of the system, acting as a hub that connects binding requests to actual input configurations.

- **[`UGCFInputBindingManagerComponent`][GCFInputBindingManagerComponent]**: The central hub that receives all input binding requests. If the state is incomplete, it queues the requests via `ProcessPendingBindings()`. It executes the bindings safely immediately after the completion of the initialization sequence is confirmed by the [`UGCFInputContextComponent`][GCFInputContextComponent].

- **[`UGCFPlayerInputBridgeComponent`][GCFPlayerInputBridgeComponent]** / **[`UGCFPawnInputBridgeComponent`][GCFPawnInputBridgeComponent]**: Implements the [`IUGCFInputConfigProvider`][GCFInputConfigProvider] interface to abstract whether the input settings belong to the "Soul" or the "Body," providing them to the Manager.

### 4. Consumer Layer
The layer where game logic actually consumes the inputs.

- **[`UGCFMovementControlComponent`][GCFMovementControlComponent]** / **[`UGCFCameraControlComponent`][GCFCameraControlComponent]**: These components do not need to know "which Pawn they are controlling." They simply send a request to the [`UGCFInputBindingManagerComponent`][GCFInputBindingManagerComponent] asking to "bind movement inputs."

---

## ⚙️ Core Mechanism: Safe Asynchronous Binding Flow

The flow to ensure input binding is safely completed in this system is as follows:

1. **Binding Request (Request):** A consumer (e.g., [`UGCFMovementControlComponent`][GCFMovementControlComponent]) requests the [`UGCFInputBindingManagerComponent`][GCFInputBindingManagerComponent] to bind Input Actions.

2. **Pending (Queueing):** At this point, the Pawn data might still be loading asynchronously. The Manager does not execute the request immediately but instead **temporarily stores the request in an internal queue (PendingBindings)**.

3. **State Confirmation (Context Ready):** Once the data load and possession are completed under GFCM's management, the [`UGCFInputContextComponent`][GCFInputContextComponent] fires an event indicating that the input context is ready (`OnContextChanged`).

4. **Safe Execution (Execute):** The Manager receives the notification and safely executes the addition of Mapping Contexts to the `UEnhancedInputLocalPlayerSubsystem` and the actual delegate bindings to the [`UGCFInputComponent`][GCFInputComponent] all at once.

---

## 🎯 Benefits of This Design

- **High Robustness and Crash Resistance** Even in asynchronous environments or high-latency networks, crashes due to Null pointer references are suppressed at the design level.

- **Realization of Plug-and-Play** When adding new vehicles or characters, the system can be expanded solely with DataAssets without modifying C++ code. By simply setting a new InputConfig in [`UGCFPawnData`][GCFPawnData], the system automatically establishes the routing.

- **Dynamic Input Cleanup** When possession ends, the Manager calls `ClearBindingsOnContextChange()` to cleanly remove the inputs of the old Body, preventing input ghosts (bugs where processes run despite the inability to control the actor).


[GCFPawnData]:                      ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Data/GCFPawnData.h

[GCFContextBinder]:                 ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/System/Binder/GCFContextBinder.h
[GCFPawnReadyStateComponent]:       ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/System/Lifecycle/GCFPawnReadyStateComponent.h
[GCFPlayerReadyStateComponent]:     ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/System/Lifecycle/GCFPlayerReadyStateComponent.h

[GCFInputContextComponent]:         ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputContextComponent.h
[GCFInputComponent]:                ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputComponent.h
[GCFInputConfig]:                   ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputConfig.h
[GCFInputBindingManagerComponent]:  ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputBindingManagerComponent.h
[GCFPlayerInputBridgeComponent]:    ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFPlayerInputBridgeComponent.h
[GCFPawnInputBridgeComponent]:      ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFPawnInputBridgeComponent.h
[GCFInputConfigProvider]:           ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputConfigProvider.h

[GCFMovementControlComponent]:      ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/GCFMovementControlComponent.h

[GCFCameraControlComponent]:        ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Camera/GCFCameraControlComponent.h