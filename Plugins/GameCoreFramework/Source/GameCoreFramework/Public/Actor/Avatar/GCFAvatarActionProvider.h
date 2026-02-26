// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GCFAvatarActionProvider.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

UINTERFACE(MinimalAPI, Blueprintable)
class UGCFAvatarActionProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief Interface to pull avatar-specific action intents (Jump, Crouch) from a Pawn.
 * Used primarily by Mover Input Producers to securely read cached states without casting.
 */
class UE_API IGCFAvatarActionProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Action")
	bool GetIsJumpPressed() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Action")
	bool GetIsJumpJustPressed() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Action")
	void ConsumeJumpJustPressed();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GCF|Action")
	bool GetWantsToCrouch() const;
};

#undef UE_API