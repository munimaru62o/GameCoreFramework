// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayTagContainer.h"

class UWorld;

/**
 * @brief RAII Wrapper for Gameplay Message Subscriptions.
 *
 * Automatically unregisters the listener when this object goes out of scope.
 * Essential for preventing crashes caused by dangling listeners in the subsystem.
 */
class FGCFMessageSubscription
{
public:
	FGCFMessageSubscription() = default;

	FGCFMessageSubscription(UWorld* World, FGameplayMessageListenerHandle InHandle);

	/** Destructor ensures the listener is unregistered. */
	~FGCFMessageSubscription() { Unsubscribe(); }

	/** Manually unregister the listener. */
	void Unsubscribe();

	// Disable Copy to prevent double-free issues.
	FGCFMessageSubscription(const FGCFMessageSubscription&) = delete;
	FGCFMessageSubscription& operator=(const FGCFMessageSubscription&) = delete;

	// Enable Move Construction.
	FGCFMessageSubscription(FGCFMessageSubscription&& Other) noexcept;

	// Enable Move Assignment (with safety fix).
	FGCFMessageSubscription& operator=(FGCFMessageSubscription&& Other) noexcept;

private:
	TWeakObjectPtr<UWorld> WeakWorld;
	FGameplayMessageListenerHandle Handle;
};