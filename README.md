# GameCoreFramework (GCF)

![Unreal Engine](https://img.shields.io/badge/Unreal_Engine-5.7+-white.svg?logo=unrealengine&logoColor=white&color=0E1128)
![License](https://img.shields.io/badge/License-MIT-green.svg)

*Read this in other languages: [English](README.md), [日本語 (Japanese)](README_ja.md)*

## 🎯 Project Overview

This project is a highly extensible, loosely-coupled, modular game framework for Unreal Engine 5. It is built upon the modern architecture of the **Lyra Starter Game** and the design philosophies of the **Gameplay Ability System (GAS)**.

---

## 💡 Core Design Philosophy

- **Complete Separation of Concerns**  
Eliminates hardcoding within `ACharacter` or `APlayerController` by dividing features into independent components (GameFeatures). This allows developers to safely inject new features and characters without polluting or breaking the existing codebase.

- **Separation of "Soul" and "Body"**  
Strictly separates the responsibilities of the `PlayerState` (the persistent "Soul") and the `Pawn` (the temporary "Body"). This architecture allows abilities and input bindings to be cleanly categorized into persistent elements and temporary, body-specific elements.

- **Data-Driven Routing**  
Constructed a routing layer driven by `DataAssets`, enabling non-programmers (designers and artists) to bind inputs to abilities and tweak behaviors without touching a single line of C++. This maximizes the team's iteration speed.

- **Safe Asynchronous Lifecycle Management**  
Utilizes the `GameFrameworkComponentManager` (GFCM) to strictly manage component dependencies and initialization states (Feature States). By implementing a hybrid approach of event-driven and state management, the system ensures synchronization of the latest state at all times, systemically preventing UE's notorious initialization order issues (e.g., referencing unspawned objects).

- **Performance-Oriented Tickless Design**  
Fundamentally eliminates reliance on per-frame `Tick` processing by default. The system operates entirely on an event-driven model and state broadcasts. This drastically cuts idle CPU overhead, delivering overwhelming performance by keeping Game Thread processing times down to a few milliseconds.

---

## 📚 Detailed Documentation

For a deep dive into the GCF design philosophy and individual systems, please refer to the following documents:

- **[Architecture Overview](Document/en/Architecture/overview.md)**
  - Summarizes the core philosophy (e.g., Separation of Soul and Body) to prevent "initialization race conditions" and "bloated responsibilities" common in multiplayer development, along with lessons learned from practical failures.

- **[GAS Integration & Ability Routing (Dual ASC & Router Pattern)](Document/en/Architecture/ability_system.md)**
  - Details the advanced "Dual ASC Architecture" where both the PlayerState and the Pawn possess their own Ability System Component. Explains the routing mechanism that eliminates tight coupling between inputs and abilities, dynamically dispatching inputs based on tag prefixes.

- **[Input System (InputBridge & Manager Pattern)](Document/en/Architecture/input_system.md)**
  - A robust manager design that queues binding requests until all contexts are ready, applying them simultaneously at a safe timing. This completely eradicates input binding crashes caused by asynchronous loading during possession.

- **[Actor Control System (Interface-Driven Control System)](Document/en/Architecture/control_system.md)**
  - Completely decouples the "player's intended input" from the "Pawn's physical behavior and unique actions" using Interfaces. This prevents casting hell and loosely handles not only movement but also vehicle-specific actions (e.g., jumping, turning on headlights).

---

## 🚀 Quick Start (Demo Environment Setup)

This repository is structured as an all-in-one package, containing both the core framework (plugin) and a sample project where you can immediately test its behavior.

To maximize extensibility and robustness, this framework relies on the latest foundational plugins included in Epic Games' "Lyra Starter Game". Please follow the steps below to set up the demo environment.

### Step 1: Clone the Repository

1. Clone this repository to your local machine, or download and extract the ZIP file.

### Step 2: Port Lyra Dependencies (Required)

To compile and run this framework, you must download the **Lyra Starter Game (UE5.7 compatible version)** from the Epic Games Launcher and copy specific plugins into this repository.

1. Open the Lyra Starter Game project directory (`[LyraProjectDirectory]/Plugins/`).

2. Copy the following Lyra-specific plugin folders into the `Plugins` folder of this cloned repository:

  - CommonGame
  - CommonUser
  - CommonLoadingScreen
  - GameplayMessageRouter
  - ModularGameplayActors
  - GameSubtitles
  - UIExtension

*(Note: Built-in engine plugins like `GameFeatures` and `Mover` do not need to be copied as they are already included in the engine).*

▼ Final Directory Structure
```text
GameCoreFramework/ (Cloned repository root)
 ├── GCF_SampleProject.uproject
 ├── Source/
 └── Plugins/
     ├── GameCoreFramework/      <-- This framework (Included by default)
     ├── CommonGame/             <-- 📥 Copied from Lyra
     ├── GameplayMessageRouter/  <-- 📥 Copied from Lyra
     └── ... (Other copied plugins)
```

### Step 3: Build the Project

1. Right-click the `GCF_SampleProject.uproject` file in the repository root and select **"Generate Visual Studio project files"**.

2. Open the generated `.sln` (or your IDE's project file) and build the project (e.g., Development Editor).

3. Once the editor launches, go to `Edit > Plugins` and ensure that this plugin and its dependencies (`GameplayAbilities`, `EnhancedInput`, `Mover`, etc.) are Enabled.

---

### Integrating into Your Own Project

If you wish to integrate this framework into your own game project, simply copy the `Plugins/GameCoreFramework` folder from this repository, along with the Lyra dependency plugins prepared in `Step 2`, directly into your own project's `Plugins` folder.


## 🎬 Demos & Samples

This project includes demo assets to help you understand the framework's behavior.  
The design allows you to dynamically switch abilities and behaviors simply by editing `DataAssets` without modifying any C++ code.

```text
GameCoreFramework/Content/Sample
├── Assets/         # Assets for Pawns, Particles, etc.
├── Blueprints/     # Blueprint classes for Pawns and Abilities
├── DataAssets/     # Core data-driven definitions
├── Experiences/    # Experience definitions
├── Maps/           # Demo map definitions
└── UI/             # Debug HUD UI definitions
```

### 🖥️ Sample Video  

https://github.com/user-attachments/assets/8a15b152-5af3-4229-9f05-af8823d93742

#### **💡 Video Highlights**
You can observe the "perfect synchronization of lifecycles": the moment possession changes, the old body's input bindings are safely discarded, **the new Pawn's `InputBinding` is dynamically updated**, and the abilities granted to the new body are immediately activated and routed.

#### 📊 On-Screen Debug Information
- **Debug Input Info:** Real-time display of currently active input actions and their routing status.
- **Debug State Info:** Monitors GFCM's `InitState` (initialization phases of each feature) and current possession status.
- **Debug Log:** Outputs possession switch events and tag transmission logs routed via the Ability Router.

#### 🎮 Executing Possession and Target Selection
- Possession is transferred by executing the `Interact` ability (a persistent ability of the "Soul") towards any highlighted target Actor.
- The target is dynamically selected based on the camera mode (center of the screen in TPS mode, or based on the mouse cursor position in Top-Down mode).

#### 🤖 Playable Actor Variations (Coexistence of New/Old Systems and Physics Engines)
To prove the architecture's loose coupling, the framework seamlessly transitions between Pawns equipped with entirely different physics components:
- **White Mannequin:** Uses the standard `CharacterMovementComponent` with Jump/Crouch functions.
- **Colored Mannequin:** Adds execution functions for dedicated abilities defined specifically for the current "body", in addition to the white mannequin's features.
- **Sphere (Mover):** Tick-based movement control via UE5's next-gen `Mover` plugin, paired with a Top-Down camera.
- **Vehicle (Chaos Vehicle):** Full vehicle control via `ChaosVehicleMovementComponent`, including unique actions like Headlights and Handbrake.

### 🌐 Client Sample Video (Network Lag Simulation)  

https://github.com/user-attachments/assets/212a420c-ef0d-4caa-8247-e3068315acb4

#### **💡 Video Highlights**
Even in poor network environments where "reversed initialization orders" or "lifecycle discrepancies during asynchronous loading (race conditions)" occur due to lag, you can see how GFCM's strict state management perfectly absorbs these issues.

#### 📡 Test Environment (Intentional Network Latency)
Simulates severe network constraints common in multiplayer games on a Client connected to a Listen Server.
- **Latency:** 200 ms
- **Packet Loss:** 10%

Even under such severe lag conditions, the possession transition completes safely. Null reference crashes and input binding failures are completely prevented, proving the architecture's robust functionality.

---

## 📌 Project Status & Contribution Policy

This project is published primarily as a **reference implementation and for educational purposes** to demonstrate a modern architecture design in Unreal Engine 5.

Therefore, please note that I am **not actively accepting Pull Requests (PRs)** for new features, mechanical changes, or major structural overhauls. (Minor bug fixes or typo corrections are appreciated, but please consider opening an issue to discuss them before submitting a PR).

If you find the design philosophy of this framework useful, please feel free to fork this repository, or simply take the architectural concepts and code snippets to build your own robust projects under the MIT License!

---

## 💖 Credits & Acknowledgments

- **[Lyra Starter Game](https://dev.epicgames.com/documentation/en-us/unreal-engine/lyra-sample-game-in-unreal-engine)** by Epic Games: 
  Heavily inspired the application of modular design philosophies.
- **Unreal Engine Community**: 
  Thanks to all developers who share their best practices.

---

## ⚖ License

This project is licensed under the MIT License.
See the [LICENSE](LICENSE) file for details.  
*Inspired by Lyra Starter Game by Epic Games.*