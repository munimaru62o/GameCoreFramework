// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Messages/GCFMessageTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"


FGCFMessageSubscription::FGCFMessageSubscription(UWorld* World, FGameplayMessageListenerHandle InHandle)
	: WeakWorld(World)
	, Handle(InHandle)
{};


FGCFMessageSubscription::FGCFMessageSubscription(FGCFMessageSubscription&& Other) noexcept
	: WeakWorld(Other.WeakWorld)
	, Handle(Other.Handle)
{
	// Invalidate the source handle so it doesn't unsubscribe on destruction
	Other.Handle = FGameplayMessageListenerHandle();
	Other.WeakWorld = nullptr;
}


FGCFMessageSubscription& FGCFMessageSubscription::operator=(FGCFMessageSubscription&& Other) noexcept
{
	if (this != &Other) {
		// [Important] Unsubscribe existing handle before overwriting it to prevent listener leaks.
		Unsubscribe();

		WeakWorld = Other.WeakWorld;
		Handle = Other.Handle;

		// Invalidate the source
		Other.Handle = FGameplayMessageListenerHandle();
		Other.WeakWorld = nullptr;
	}
	return *this;
}


void FGCFMessageSubscription::Unsubscribe()
{
	if (Handle.IsValid()) {
		if (UWorld* World = WeakWorld.Get()) {
			UGameplayMessageSubsystem& Subsystem = UGameplayMessageSubsystem::Get(World);
			Subsystem.UnregisterListener(Handle);
		}

		// Invalidate handle to prevent double unregistration
		Handle = FGameplayMessageListenerHandle();
	}
	WeakWorld.Reset();
}