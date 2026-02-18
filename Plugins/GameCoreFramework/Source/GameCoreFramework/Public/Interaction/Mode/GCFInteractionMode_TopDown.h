// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#include "Interaction/Mode/GCFInteractionMode.h"
#include "GCFInteractionMode_TopDown.generated.h"

/**
 * @brief Interaction mode for Top-Down or Isomeric views (Mouse-driven).
 * * Deprojects the mouse cursor position to the world and performs a trace under the cursor.
 */
UCLASS()
class GAMECOREFRAMEWORK_API UGCFInteractionMode_TopDown : public UGCFInteractionMode
{
	GENERATED_BODY()
public:
	virtual AActor* FindTarget(const FGCFInteractionSearchParams& Params) const override;
};

