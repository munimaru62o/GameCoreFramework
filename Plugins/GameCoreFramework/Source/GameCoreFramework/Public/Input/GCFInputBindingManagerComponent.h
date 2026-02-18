// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "Input/GCFInputTypes.h"
#include "System/Lifecycle/GCFStateTypes.h"
#include "System/Binder/GCFContextBinder.h"
#include "GCFInputBindingManagerComponent.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class UGCFInputComponent;
class IGCFInputConfigProvider;
class FGCFDelegateHandle;
class UInputMappingContext;

/**
 * Delegate responsible for executing the actual input binding logic.
 * Expects the callee to bind input actions to the provided InputComponent and return a list of receipts.
 */
DECLARE_DELEGATE_RetVal_TwoParams(
	TArray<FGCFBindingReceipt>,
	FGCFInputBindNativeDelegate,
	UGCFInputComponent*,
	TScriptInterface<IGCFInputConfigProvider>
);

/**
 * @brief Centralized manager for Input Binding lifecycle and execution.
 *
 * This component acts as the orchestrator for Enhanced Input bindings within the Controller.
 * It solves the "Input Dependency" problem where inputs must not be bound until specific conditions are met
 * (e.g., Pawn is possessed, AbilitySystem is ready, InputComponent is initialized).
 *
 * [Key Features]
 * - Late Binding: Queues binding requests until the "Body" (Pawn) and "Soul" (Player) are fully ready.
 * - Context Awareness: Distinguishes between Pawn-dependent bindings (cleared on unpossession)
 * and Controller-persistent bindings (survive pawn swaps).
 * - Safety: Prevents duplicate bindings via idempotency checks and ensures clean removal upon context changes.
 */
UCLASS(MinimalAPI, ClassGroup = (GCF), Within = PlayerController, HideCategories = (Tags, Activation, Cooking, AssetUserData, Collision, Networking, Replication), meta = (BlueprintSpawnableComponent, CollapseCategories))
class UGCFInputBindingManagerComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UGCFInputBindingManagerComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * Static entry point to register an input binding request.
	 *
	 * [Usage Warning]
	 * If the Context belongs to a Pawn, this should be called AFTER possession (e.g., inside OnPawnReady).
	 * If the Controller cannot be resolved from the Context, the request is ignored.
	 *
	 * @param Context   The object requesting the binding. Used to determine lifecycle dependency (Pawn vs Controller).
	 * @param KeyName   A unique identifier for this binding (e.g., function name) to prevent duplication.
	 * @param Delegate  The logic to execute the binding and return receipts.
	 */
	UE_API static void RegisterInputBinding(UObject* Context, FName KeyName, FGCFInputBindNativeDelegate&& Delegate);

	/**
	 * Notifies the manager that binding conditions may have changed (e.g., InputConfig added).
	 */
	UE_API void NotifyBindingContextChanged();

	/** Logs all active and pending bindings for debugging. */
	UE_API void DumpInputBindings();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Unique key to identify a pending binding request. */
	struct FGCFPendingBindingKey
	{
		TWeakObjectPtr<UObject> Binder;
		FName KeyName;

		bool operator==(const FGCFPendingBindingKey& Other) const
		{
			return Binder == Other.Binder && KeyName == Other.KeyName;
		}

		// Standard UE hash implementation for structs used in TMap.
		friend uint32 GetTypeHash(const FGCFPendingBindingKey& Key)
		{
			// Combine the hash of the Binder (WeakPtr) and the KeyName.
			return HashCombine(GetTypeHash(Key.Binder), GetTypeHash(Key.KeyName));
		}
	};

	/** Internal method to queue binding requests. */
	void RegisterInputBinding_Internal(UObject* Context, FName KeyName, FGCFInputBindNativeDelegate&& Delegate);

	void HandlePawnPossessionStateChanged(AActor* Actor, bool bPossessed);
	void HandleInputComponentReady(UInputComponent* InputComp);
	void HandleInputContextStateChanged(EGCFInputContextState CurrentState, bool bIsEnable);

	/** Evaluates whether bindings can be processed based on the current state. */
	void EvaluateBindingConditions();

	/** Processes the PendingBindings queue when conditions (Pawn Ready & Input Ready) are met. */
	void ProcessPendingBindings();

	/** 
	 * Reads Input Mapping Contexts (IMCs) from the target Pawn's data
	 * and adds them to the Enhanced Input Local Player Subsystem.
	 */
	void ApplyPawnMappingContexts(AActor* PawnActor);

	/** 
	 * Removes all currently applied Pawn-dependent IMCs from the Subsystem.
	 */
	void ClearPawnMappingContexts();

	/** 
	 * Determines if a binder object belongs to the Pawn's lifecycle.
	 * Used to decide whether to clear bindings when possession changes.
	 */
	virtual bool IsBinderPawnDependent(UObject* Binder) const;

	/**
	 * Executes the delegate and registers the receipts to ActiveBindingGroups.
	 * Includes checks to prevent duplicate bindings on the same InputComponent.
	 */
	virtual void ExecuteInputBinding(const FGCFPendingBindingKey& Key, const FGCFInputBindNativeDelegate& Delegate, UGCFInputComponent* InputComp, TScriptInterface<IGCFInputConfigProvider> Provider);

	/** 
	 * Clears bindings associated with the Pawn when possession is lost.
	 * Controller-persistent bindings are preserved.
	 */
	void ClearBindingsOnContextChange();

	void SendDebugInfo();

private:
	bool bInputEnabled = false;

	/** Lifecycle management handles for delegates. */
	TUniquePtr<FGCFDelegateHandle> InputContextTrackerHandle;
	TUniquePtr<FGCFDelegateHandle> InputComponentReadyHandle;
	TUniquePtr<FGCFContextBinder> PawnPossessionBinder;

	/** 
	 * List of currently applied Pawn-dependent IMCs.
	 * Tracked here to ensure clean removal upon unpossession.
	 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<const UInputMappingContext>> AppliedPawnIMCs;

	/** Queue of binding requests waiting for conditions to be met. */
	TMap<FGCFPendingBindingKey, FGCFInputBindNativeDelegate> PendingBindings;

	/** Represents a physically active binding group in the Enhanced Input System. */
	struct FGCFInputBindingGroup
	{
		FGCFPendingBindingKey Key;
		TArray<FGCFBindingReceipt> Receipts;
		bool bIsPawnDependent = false;

		/** The specific InputComponent instance where bindings were applied. Used for safety checks. */
		TWeakObjectPtr<UInputComponent> BoundInputComponent;

#if !UE_BUILD_SHIPPING
		TArray<FString> DebugBindingInfos;
#endif
	};
	/** List of currently active bindings. Target for clearing during context changes. */
	TArray<FGCFInputBindingGroup> ActiveBindingGroups;
};


#undef UE_API