// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"

#include "GCFShared.h"
#include "Components/ControllerComponent.h"
#include "GCFPossessionContextComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class APawn;
class AController;
class FGCFDelegateHandle;

/**
 * @brief Component that bridges "Physical Possession" changes to "Gameplay Events".
 *
 * [Responsibilities]
 * 1. Listens to possession changes via GCFControllerPossessionComponent.
 * 2. Updates "Feature States" on the Pawn and PlayerState via GameFrameworkComponentManager.
 * 3. Broadcasts Gameplay Events (OnPossessed/OnUnpossessed) to other systems.
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = Controller, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision, Networking, Replication), meta = (BlueprintSpawnableComponent, CollapseCategories))
class UGCFPossessionContextComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UE_API UGCFPossessionContextComponent(const FObjectInitializer& ObjectInitializer);

	void CheckAndUpdatePlayerPossessionState();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Callback function executed when the Controller's possession changes. */
	void HandlePawnChanged(APawn* OldPawn, APawn* NewPawn);

	/** Helper to send extension events via GFCM. */
	void BroadcastEvent(APawn* Pawn, FName EventName);

private:
	/** Scoped handle to automatically unbind from the possession tracker. */
	TUniquePtr<FGCFDelegateHandle> PossessionTrackerHandle;
};

#undef UE_API