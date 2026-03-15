# GameCoreFramework (GCF)

![Unreal Engine](https://img.shields.io/badge/Unreal_Engine-5.7+-white.svg?logo=unrealengine&logoColor=white&color=0E1128)
![Version](https://img.shields.io/badge/Version-0.9.0_Beta-blue.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

**Code Quality:**  
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/81b2b1c82dd345e0abb35dbef49c258b)](https://app.codacy.com/gh/munimaru62o/GameCoreFramework/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=munimaru62o_GameCoreFramework&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=munimaru62o_GameCoreFramework)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=munimaru62o_GameCoreFramework&metric=reliability_rating)](https://sonarcloud.io/summary/new_code?id=munimaru62o_GameCoreFramework)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=munimaru62o_GameCoreFramework&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=munimaru62o_GameCoreFramework)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=munimaru62o_GameCoreFramework&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=munimaru62o_GameCoreFramework)

🌍 *Read this in other languages: [English](README.md) | [日本語 (Japanese)](README_ja.md)* *(Note: The English documentation is AI-translated from the original Japanese).*

## 📑 Table of Contents
- [🎯 Project Overview](#-project-overview)
- [💡 Core Design Philosophy](#-core-design-philosophy)
- [📚 Detailed Documentation](#-detailed-documentation)
- [🚀 Quick Start (Installation Guide)](#-quick-start-installation-guide)
  - [Step 1: Clone the Repository](#step-1-clone-the-repository)
  - [Step 2: Transplant Lyra Dependency Plugins (Required)](#step-2-transplant-lyra-dependency-plugins-required)
  - [Step 3: Build the Project](#step-3-build-the-project)
  - [Integrating into Your Own Project](#integrating-into-your-own-project)
- [🎬 Demo & Samples](#-demo--samples)
- [📌 Project Status and Contribution](#-project-status-and-contribution)
- [💖 Credits & Acknowledgments](#-credits--acknowledgments)
- [🤖 AI Usage Policy](#-ai-usage-policy)
- [⚖ License](#-license)

## 🎯 Project Overview

This project is a highly extensible, loosely coupled modular game framework built upon the modern architecture of Unreal Engine 5 (inspired by Lyra Starter Game) and the design philosophy of the Gameplay Ability System (GAS).

---

## 💡 Core Design Philosophy

- **Clear Separation of Concerns**
  By eliminating hardcoding in `ACharacter` and `APlayerController`, each feature is divided into independent components (GameFeatures). This allows you to safely add and expand new features and characters without polluting the existing codebase.

- **Separation of Soul and Body**
  The responsibilities of the PlayerState (the persistent "Soul") and the Pawn (the temporary "Body") are clearly separated. This makes it possible to decompose Ability and Input bindings into persistent and temporary ones, achieving a flexible system that is not dictated by the current possession state.

- **Data-Driven Routing**
  A routing layer is established via DataAssets, enabling non-programmers (planners and artists) to link inputs with abilities and adjust behaviors without touching C++ code. This maximizes the iteration speed of the entire team.

- **Safe Asynchronous Lifecycle Management**
  Utilizing the `GameFrameworkComponentManager` (GFCM), the dependencies and initialization states (Feature States) of each component are strictly managed. By implementing a hybrid architecture of event-driven and state management, the system can always synchronize to the latest state. This prevents at the system level the frequent Unreal Engine initialization order issue where "referenced objects have not yet been spawned."

- **Performance-Oriented Tickless Design**
  Reliance on per-frame `Tick` processing is fundamentally eliminated. The system operates primarily through an Event-Driven model and state broadcasting. This suppresses CPU overhead on the Game Thread, maintaining high runtime performance.

---

## 📚 Detailed Documentation

> 💡 **Reference Article: Architecture Background**  
> Before diving into the code, I highly recommend reading the following technical article (originally in Japanese). It explains the core design philosophy of this framework—**"Loose coupling to enable emergent, unpredictable gameplay."** It covers the severe trade-offs encountered during development and the resulting architectural solutions  
> 🔗 [Why I am Obsessed with Loose Coupling and Component-Oriented Design in Game Development](https://zenn.dev/munimaru62o/articles/c6ed730c6e4c61?locale=en)

For details on the design philosophy and individual systems of GCF, please refer to the following documentation:

- **[Architecture Overview](Document/en/Architecture/overview.md)**
  Summarizes the core philosophy (e.g., Separation of Soul and Body) to systematically prevent "initialization race conditions" and "responsibility bloat" that frequently occur in multiplayer development, along with lessons learned from practical failures.

- **[GAS Integration and Ability Routing (Dual ASC & Router Pattern)](Document/en/Architecture/ability_system.md)**
  Explains the advanced "Dual ASC Architecture" where both the PlayerState and the Pawn have an ASC. It details the routing mechanism that eliminates tight coupling between inputs and abilities, dynamically switching the execution target based on tag prefixes.

- **[Input System (InputBridge & Manager Pattern)](Document/en/Architecture/input_system.md)**
  Details a robust manager design that queues requests until the state is ready and applies them safely all at once. This prevents input binding crashes caused by asynchronous loading during possession.

- **[Actor Control System (Interface-Driven Intent Dispatch & Opt-In Design)](Document/en/Architecture/control_system.md)**
  Explains the highly decoupled architecture based on an "Intent Bucket Relay." It clearly separates the player's "movement intent" from the Pawn's "physical behavior" using interfaces, where the input side Pushes the intent, the Pawn Caches it, and the physics engine (like Mover) Pulls and translates it.

---

## 🚀 Quick Start (Installation Guide)

This repository integrates the core framework (Plugin) and a sample project so you can immediately verify its behavior.

To maximize extensibility and robustness, this framework depends on the latest foundation plugins included in Epic Games' official "Lyra Starter Game." Please build the demo environment by following the steps below.

### Step 1: Clone the Repository

1. Clone this repository locally or download and extract the ZIP file.

### Step 2: Transplant Lyra Dependency Plugins (Required)

To compile and run this framework, you must download the "Lyra Starter Game (UE5.7 compatible version)" from the Epic Games Launcher and copy specific plugins into this repository.

1. Open the Lyra Starter Game project folder (`[LyraProjectDirectory]/Plugins/`).
2. Copy the following Lyra-specific plugin folders into the `Plugins` folder of your cloned repository:

   - CommonGame
   - CommonUser
   - CommonLoadingScreen
   - GameplayMessageRouter
   - ModularGameplayActors
   - GameSubtitles
   - UIExtension

*(Note: Engine plugins like GameFeatures and Mover are already built into the UE engine and do not need to be copied)*

▼ Example of the final directory structure
```text
GameCoreFramework/ (Root of the cloned repository)
 ├── GCF_SampleProject.uproject
 ├── Source/
 └── Plugins/
     ├── GameCoreFramework/      <-- This framework (Included by default)
     ├── CommonGame/             <-- 📥 Copied from Lyra
     ├── GameplayMessageRouter/  <-- 📥 Copied from Lyra
     └── ... (Other copied plugins)
```

### Step 3: Build the Project

1. Right-click `GCF_SampleProject.uproject` at the root of the repository and select "Generate Visual Studio project files."
2. Open the generated `.sln` (or your IDE's project file) and build the project (e.g., Development Editor).
3. Once the editor launches, verify from `Edit > Plugins` that this plugin and its dependencies (GameplayAbilities, EnhancedInput, Mover, etc.) are Enabled.

---

### Integrating into Your Own Project

If you wish to introduce this framework into your own game project, simply **copy the `Plugins/GameCoreFramework` folder from this repository directly into your project's `Plugins` folder**, along with the Lyra dependency plugins prepared in `Step 2`.

## 🎬 Demo & Samples

This project includes demo assets to help you verify its behavior.
It is designed so that you can dynamically switch abilities and behaviors simply by editing DataAssets, without altering any C++ code.

```text
GameCoreFramework/Content/Sample
├── Assets/         # Assets for Pawns and Particles
├── Blueprints/     # BP classes for Pawns and Abilities
├── DataAssets/     # The core of the data-driven design; DataAssets are defined here
├── Experiences/    # Experience definitions
└── Maps/           # Demo map definitions
└── UI/             # UI definitions for the Debug HUD
```

> **⚠️ Note on Experience Loading Functionality:**  
> Because this framework is built as a Minimum Viable Product (MVP) prioritizing the robustness of the core foundation, dynamic Experience switching via UI is not currently implemented. Assigning or changing the Experience is designed to be done solely via the "Default Gameplay Experience" setting within the target map's `World Settings`.

### 🖥️ Sample Video

https://github.com/user-attachments/assets/1ab546e9-f2b7-4c96-ad29-9a86f2c24af6

#### **💡 Highlights of the Video**
You can observe that the moment possession changes, the input bindings of the old Body are safely discarded, and **the new Pawn's `InputBinding` is dynamically updated.** It also demonstrates "safe synchronization of lifecycles," where Abilities granted to the new Body are immediately activated and routed.

#### 📊 Debug Information on Screen
- **Debug Input Info:** Displays currently active bound input actions and routing states in real-time.
- **Debug State Info:** Monitors GFCM's `InitState` (the initialization phase of each feature) and the current possession state.
- **Debug Log:** Outputs possession switch events and tag transmission logs via the Ability Router.

#### 🎮 Executing Possession and Target Selection
- Execute the `Interact` ability (a persistent ability on the Soul side) against an Actor highlighted (outlined) on the screen to transfer possession.
- Targets are dynamically selected based on the camera mode. (In TPS mode, it's based on the center of the screen; in Top-Down mode, it's based on the mouse cursor position).

#### 🤖 Variations of Playable Actors (Coexistence of Old/New Systems and Physics Engines)
To prove the decoupled nature of the architecture, the player seamlessly transitions between Pawns with completely different physics components.
- **White Mannequin:** Movement, Jump, and Crouch functions powered by this framework's proprietary `GCFCharacterMover`, based on UE5's next-gen `Mover` plugin.
- **Colored Mannequin:** Movement control using the legacy `CharacterMovementComponent`, plus the execution of dedicated abilities uniquely defined for the current "Body."
- **Sphere (Mover):** Tick-based movement control using the `Mover` plugin, viewed from a Top-Down camera.
- **Vehicle (Chaos Vehicle):** Authentic vehicle control using the `ChaosVehicleMovementComponent`, along with specific operations like Headlights and Handbrake.

### 🌐 Client Sample Video (Network Lag Simulation)

https://github.com/user-attachments/assets/f3c54c69-b6dc-4071-a949-bfde364a2029

#### **💡 Highlights of the Video**
You can observe that even in a poor network environment where communication delays cause "initialization order reversals" or "lifecycle desyncs during asynchronous loading (race conditions)," GFCM's strict state management safely absorbs these issues.

#### 📡 Test Environment (Intentional Network Latency)
A harsh network restriction frequently encountered in multiplayer is emulated on a Client connected to a Listen Server.
- **Latency (Ping):** 200 ms
- **Packet Loss:** 10%

It proves that the architecture functions robustly even under such severe lag conditions: possession transfers safely complete, and the occurrence of Null reference crashes or input binding failures is suppressed at the design level.

---

## 📌 Project Status and Contribution

This project is a demonstration of modern architecture design in Unreal Engine 5 and is published personally for **learning and reference purposes**.

Therefore, with the exception of bug fixes, **we generally do not accept** Pull Requests (PRs) for "adding new features" or "large-scale changes that affect the core design."

If you resonate with the design philosophy of this framework, please feel free to Fork this repository or take the code and architectural ideas back to your own projects (within the scope of the MIT License)!

---

## 💖 Credits & Acknowledgments

- **[Lyra Starter Game](https://dev.epicgames.com/documentation/en-us/unreal-engine/lyra-sample-game-in-unreal-engine)** by Epic Games:
  Significantly influenced the utilization of modular design philosophies.
- **Unreal Engine Community**:
  Thanks to all the developers who share their best practices.

---

## 🤖 AI Usage Policy

All users are fully permitted and encouraged to use AI assistants to analyze the code and documentation. However, extracting data for model training or fine-tuning is strictly prohibited.
See [AI_POLICY.md](AI_POLICY.md) for full details.

---

## ⚖ License

The original code of this project is licensed under the **MIT License**.
See [LICENSE](LICENSE) for more details.

**⚠️ Notice Regarding Epic Games Content**
This project contains code and assets inspired by or derived from the "Lyra Starter Game." These Epic Games contents are subject to the Unreal Engine End User License Agreement (EULA).
For details on which files fall under which license, please be sure to check [NOTICE.md](NOTICE.md).
