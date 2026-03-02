// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/Binder/GCFContextBinder.h"
#include "GCFShared.h"
#include "Components/GameFrameworkComponentManager.h"


FGCFContextBinder::FGCFContextBinder(UGameFrameworkComponentManager * InGFCM, const TSoftClassPtr<AActor>&InReceiverClass)
	: GFCM(InGFCM)
	, ReceiverClass(InReceiverClass)
{}


void FGCFContextBinder::Activate()
{
	if (ExtensionHandle.IsValid()) {
		return;
	}

	// 1. Fast Path: If the condition is already met, resolve immediately and bypass further monitoring.
	if (TryResolveImmediate()) {
		return;
	}

	// 2. Slow Path: Register an extension handler with the GFCM and wait for the event.
	if (GFCM.IsValid() && !ReceiverClass.IsNull()) {
		ExtensionHandle = GFCM->AddExtensionHandler(
			ReceiverClass,
			UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateRaw(this, &FGCFContextBinder::HandleExtension)
		);
	}
}


void FGCFContextBinder::Deactivate()
{
	ExtensionHandle.Reset();
}


void FGCFContextBinder::HandleExtension(AActor* Actor, FName EventName)
{
	if (Actor) {
		TryResolveEvent(Actor, EventName);
	}
}