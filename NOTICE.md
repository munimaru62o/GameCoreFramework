# Third-Party Notices

This repository contains original code written by munimaru62o under the MIT License, but it also heavily relies on code and assets provided by Epic Games, Inc.

To prevent any confusion regarding usage rights, please refer to the following guidelines based on the file type and header.

## 1. C++ Source Code (`Source/` directory)

The C++ source files (`.cpp`, `.h`) in this project fall into three categories. **Please check the top header of each file** to determine its applicable license:

* ✅ **Category A: Purely Original Code (MIT License)**
  Files containing the following header are 100% original and governed by the **MIT License**:
  ```cpp
  // Copyright (c) 2026 munimaru62o. All rights reserved.
  // Licensed under the MIT License. See LICENSE in the project root for license information.
  ```

* ⚠️ **Category B: Modified Epic Games Code (Epic Games EULA)**
  Files containing the following header are modifications of Epic Games' code (e.g., from Lyra). Because they are derivative works, they are governed by the **Unreal Engine EULA**:
  ```cpp
  // Copyright Epic Games, Inc. All Rights Reserved.
  // Portions Copyright (c) 2026 munimaru62o. All rights reserved.
  ```

* ⚠️ **Category C: Unmodified Epic Games Code (Epic Games EULA)**
  Files containing the following header are exact copies from Epic Games' templates or examples. They are strictly governed by the **Unreal Engine EULA**:
  ```cpp
  // Copyright Epic Games, Inc. All Rights Reserved.
  ```

## 2. Game Assets (`Content/` directory)

⚠️ **Covered by Epic Games EULA:**
**ALL** 3D models, skeletal meshes, animations, sounds, textures, and level blockouts located in the `Content/` directory (originating from "Lyra Starter Game", "Starter Content", and "Third Person Template") are the property of Epic Games, Inc.

### Unreal Engine End User License Agreement (EULA)

All Epic Games content (Category B code, Category C code, and all Content Assets) is subject to the Unreal Engine End User License Agreement (EULA). 
**Strict Rule:** You may **NOT** extract these assets or Epic's code for use in other game engines (like Unity or Godot). They must only be used within Unreal Engine projects.

*Copyright (c) Epic Games, Inc. All Rights Reserved.*