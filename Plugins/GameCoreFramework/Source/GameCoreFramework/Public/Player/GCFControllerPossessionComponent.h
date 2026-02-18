// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"

#include "GCFShared.h"
#include "System/Binder/GCFContextBinder.h"
#include "Components/ControllerComponent.h"
#include "GCFControllerPossessionComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class APawn;
class AController;

/**
 * @brief Component attached to AController to notify Possession/Unpossession events safely.
 *
 * [Features]
 * - Works for both PlayerController and AIController.
 * - Uses Native Multicast Delegates (No Dynamic overhead).
 * - **Safe Possession**: Delays the notification until the Pawn has fully acknowledged the Controller
 * (i.e., until Pawn->GetController() is valid).
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = Controller, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision, Networking, Replication), meta = (BlueprintSpawnableComponent, CollapseCategories))
class UGCFControllerPossessionComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UE_API UGCFControllerPossessionComponent(const FObjectInitializer& ObjectInitializer);

	/** Helper to find this component on an Actor (Controller). */
	UFUNCTION(BlueprintPure, Category = "GCF|Possession")
	static UGCFControllerPossessionComponent* FindGCFControllerPossessionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UGCFControllerPossessionComponent>() : nullptr); }

	/**
	 * Registers a delegate and optionally executes it immediately with the current state.
	 * @return Handle to unregister later.
	 */
	UE_API FDelegateHandle RegisterAndExecuteDelegate(const FOnPossessedPawnChangedNative::FDelegate& Delegate, bool bExecuteImmediately = true);

	UE_API void RemoveDelegate(const FDelegateHandle& Handle);

	/** Returns the current "Confirmed" Pawn (Source of Truth). */
	UE_API APawn* GetCurrentPawn() const { return CachedPawn.Get(); }

	/** Called from the owning Controller's OnNewPawnNotifier. */
	void HandleNewPawn(APawn* NewPawn);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void InitNewPawnHandle(AController* Controller);

	/** Syncs initial state for late-join handling. */
	void SyncInitialPawn();

	/** Callback from the Binder when the PendingPawn is fully ready (Controller Assigned). */
	void HandlePawnControllerChanged(AActor* Actor, bool bControllerAssigned);

	/** Commits the new pawn and broadcasts the change. */
	void SetNewPawn(APawn* NewPawn);


private:
	/** The Pawn that is fully linked and confirmed. */
	TWeakObjectPtr<APawn> CachedPawn;

	/** The Pawn that has been possessed by Controller, but hasn't acknowledged the Controller yet. */
	TWeakObjectPtr<APawn> PendingPawn;

	TUniquePtr<FGCFDelegateHandle> NewPawnHandle;
	TUniquePtr<FGCFContextBinder> PendingPawnReadyHandle;

	FOnPossessedPawnChangedNative OnPawnChangedNative;
};


#undef UE_API