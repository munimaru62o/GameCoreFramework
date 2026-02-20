// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Player/GCFControllerPossessionComponent.h"

#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "System/Binder/GCFPawnControllerAssignedBinder.h"
#include "Components/GameFrameworkComponentManager.h"


UGCFControllerPossessionComponent::UGCFControllerPossessionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}


FDelegateHandle UGCFControllerPossessionComponent::RegisterAndExecuteDelegate(const FOnPossessedPawnChangedNative::FDelegate& Delegate, bool bExecuteImmediately)
{
	if (bExecuteImmediately) {
		// Even if CachedPawn is null, we might want to notify "Current is Null" depending on usage.
		// Usually, we only notify if we have a valid pawn, OR if the system expects an immediate callback regardless.
		if (CachedPawn.IsValid()) {
			Delegate.ExecuteIfBound(nullptr, CachedPawn.Get());
		}
	}
	return OnPawnChangedNative.Add(Delegate);
}


void UGCFControllerPossessionComponent::RemoveDelegate(const FDelegateHandle& Handle)
{
	OnPawnChangedNative.Remove(Handle);
}


void UGCFControllerPossessionComponent::BeginPlay()
{
	Super::BeginPlay();

	AController* Controller = Cast<AController>(GetOwner());
	if (!Controller) {
		return;
	}

	InitNewPawnHandle(Controller);

	// Sync initial state
	SyncInitialPawn();
}

void UGCFControllerPossessionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	NewPawnHandle.Reset();
	PendingPawnReadyHandle.Reset();
	CachedPawn.Reset();
	PendingPawn.Reset();

	Super::EndPlay(EndPlayReason);
}


void UGCFControllerPossessionComponent::InitNewPawnHandle(AController* Controller)
{
	if (Controller) {
		FDelegateHandle RawHandle = Controller->GetOnNewPawnNotifier().AddUObject(this, &ThisClass::HandleNewPawn);

		TWeakObjectPtr<AController> ControllerPtr(Controller);
		NewPawnHandle = MakeUnique<FGCFDelegateHandle>([ControllerPtr, RawHandle]() {
			if (ControllerPtr.IsValid()) {
				ControllerPtr->GetOnNewPawnNotifier().Remove(RawHandle);
			}
		});
	}
}


void UGCFControllerPossessionComponent::SyncInitialPawn()
{
	if (AController* Controller = GetController<AController>()) {
		APawn* CurrentPawn = Controller->GetPawn();
		HandleNewPawn(CurrentPawn);
	}
}


void UGCFControllerPossessionComponent::HandleNewPawn(APawn* NewPawn)
{
	if (PendingPawn == NewPawn) {
		return;
	}

	// We don't need to wait for "Controller Assigned" on a null pawn.
	if (!NewPawn) {
		PendingPawn = nullptr;
		PendingPawnReadyHandle.Reset();
		SetNewPawn(nullptr);
		return;
	}

	// [Logic] NewPawn is valid. Wait until the Pawn fully acknowledges us.
	if (UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(GetOwner())) {
		PendingPawn = NewPawn;

		// Create a binder that waits for the Pawn's extension event indicating it has received the controller.
		// Note: FGCFPawnControllerAssignedBinder should check "Pawn->GetController()" internally or wait for the event.
		PendingPawnReadyHandle = FGCFPawnControllerAssignedBinder::CreateBinder(
			GFCM,
			PendingPawn.Get(),
			FGCFBooleanStateSignature::CreateUObject(this, &ThisClass::HandlePawnControllerChanged)
		);
	} else {
		// Fallback: If GFCM is missing (rare), commit immediately to avoid getting stuck.
		SetNewPawn(NewPawn);
	}
}


void UGCFControllerPossessionComponent::HandlePawnControllerChanged(AActor* Actor, bool bControllerAssigned)
{
	// Ensure we are talking about the pending pawn
	if (bControllerAssigned && PendingPawn == Actor) {
		SetNewPawn(PendingPawn.Get());
		PendingPawn = nullptr;
		PendingPawnReadyHandle.Reset();
	}
}


void UGCFControllerPossessionComponent::SetNewPawn(APawn* NewPawn)
{
	APawn* OldPawn = CachedPawn.Get();
	if (OldPawn == NewPawn) {
		return;
	}

	UE_LOG(LogGCFSystem, Log, TEXT("[%s] UGCFControllerPossessionComponent: SetNewPawn: %s"),
		   *GetClientServerContextString(this), *GetNameSafe(NewPawn));

	// [Important] Update State BEFORE Broadcast.
	// This ensures that if a delegate calls GetCurrentPawn(), they get the NEW pawn.
	CachedPawn = NewPawn;
	OnPawnChangedNative.Broadcast(OldPawn, NewPawn);
}