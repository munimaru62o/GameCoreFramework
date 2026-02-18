// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Input/GCFInputBindingManagerComponent.h"

#include "GCFShared.h"
#include "Actor/Data/GCFPawnData.h"
#include "Actor/Data/GCFPawnDataProvider.h"
#include "Actor/GCFActorFunctionLibrary.h"
#include "Player/GCFPlayerController.h"
#include "Input/GCFInputComponent.h"
#include "Input/GCFInputFunctionLibrary.h"
#include "Input/GCFInputConfigProvider.h"
#include "EnhancedInputSubsystems.h"
#include "Components/GameFrameworkComponentManager.h"
#include "System/Binder/GCFControllerPossessionBinder.h"
#include "System/Lifecycle/GCFPawnReadyStateComponent.h"
#include "System/Lifecycle/GCFPlayerReadyStateComponent.h"

// for debugging
#include "Common/GCFDebugTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "System/GCFDebugFunctionLibrary.h"


UGCFInputBindingManagerComponent::UGCFInputBindingManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bWantsInitializeComponent = true;
}


void UGCFInputBindingManagerComponent::RegisterInputBinding(UObject* Context, FName KeyName, FGCFInputBindNativeDelegate&& Delegate)
{
	if (!Context || !Delegate.IsBound()) {
		return;
	}

	if (KeyName.IsNone()) {
		UE_LOG(LogGCFSystem, Warning, TEXT("RegisterInputBinding: Key is None for Context [%s]. Please provide a unique Key."), *Context->GetName());
	}

	if (const AController* Controller = UGCFActorFunctionLibrary::ResolveController(Context)) {
		if (UGCFInputBindingManagerComponent* Manager = Controller->FindComponentByClass<UGCFInputBindingManagerComponent>()) {
			Manager->RegisterInputBinding_Internal(Context, KeyName, MoveTemp(Delegate));
		}
	} else {
		UE_LOG(LogGCFSystem, Warning, TEXT("GCFInputBindingManager: Failed to resolve Controller for context [%s]. Input binding will be ignored. (Pawn might not be possessed yet)"), *Context->GetName());
	}
}


void UGCFInputBindingManagerComponent::RegisterInputBinding_Internal(UObject* Context, FName KeyName, FGCFInputBindNativeDelegate&& Delegate)
{
	FGCFPendingBindingKey BindingKey{ Context, KeyName };

	// Always overwrite existing pending bindings to ensure we have the latest delegate logic.
	if (!PendingBindings.Find(BindingKey)) {
		PendingBindings.Add(BindingKey, MoveTemp(Delegate));
	}
	EvaluateBindingConditions();
}


void UGCFInputBindingManagerComponent::NotifyBindingContextChanged()
{
	if (PendingBindings.IsEmpty()) {
		return;
	}
	EvaluateBindingConditions();
}


void UGCFInputBindingManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AGCFPlayerController* Controller = GetController<AGCFPlayerController>()) {
		// Only manage input for local players (Human Input).
		if (!Controller->IsLocalController()) {
			Deactivate();
			return;
		}

		// Subscribe to Input Context availability (Gatekeeper)
		InputContextTrackerHandle = UGCFInputFunctionLibrary::BindInputContextScoped(
			Controller, FOnInputContextEvaluatedNative::FDelegate::CreateUObject(this, &ThisClass::HandleInputContextStateChanged));

		// Subscribe to Input Component availability
		InputComponentReadyHandle = UGCFInputFunctionLibrary::BindInputComponentReadyScoped(
			Controller, FOnInputComponentReady::FDelegate::CreateUObject(this, &ThisClass::HandleInputComponentReady));

		// Monitor Possession changes to clean up pawn-dependent bindings
		if (UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(GetOwner())) {
			PawnPossessionBinder = FGCFControllerPossessionBinder::CreateBinder(GFCM, Controller, FGCFBooleanStateSignature::CreateUObject(this, &ThisClass::HandlePawnPossessionStateChanged));
		}
	}
}


void UGCFInputBindingManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	InputContextTrackerHandle.Reset();
	PawnPossessionBinder.Reset();

	Super::EndPlay(EndPlayReason);
}



void UGCFInputBindingManagerComponent::HandlePawnPossessionStateChanged(AActor* Actor, bool bPossessed)
{
	if (!bPossessed) {
		// When possession is lost, verify and clear only pawn-dependent bindings.
		ClearPawnMappingContexts();
		ClearBindingsOnContextChange();
	} else {
		ApplyPawnMappingContexts(Actor);
	}
	EvaluateBindingConditions();
}


void UGCFInputBindingManagerComponent::HandleInputComponentReady(UInputComponent* InputComp)
{
	if (InputComp) {
		EvaluateBindingConditions();
	}
}


void UGCFInputBindingManagerComponent::HandleInputContextStateChanged(EGCFInputContextState CurrentState, bool bIsEnable)
{
	if (bInputEnabled != bIsEnable) {
		bInputEnabled = bIsEnable;
		EvaluateBindingConditions();
	}
}


void UGCFInputBindingManagerComponent::ProcessPendingBindings()
{
	APlayerController* PC = GetController<APlayerController>();
	UGCFInputComponent* InputComp = PC ? Cast<UGCFInputComponent>(PC->InputComponent) : nullptr;
	TScriptInterface<IGCFInputConfigProvider> PlayerProvider = UGCFInputFunctionLibrary::ResolveInputConfigProvider(this, EGCFInputSourceType::Player);
	TScriptInterface<IGCFInputConfigProvider> PawnProvider = UGCFInputFunctionLibrary::ResolveInputConfigProvider(this, EGCFInputSourceType::Pawn);

	if (!InputComp || !PlayerProvider || !PawnProvider) {
		return;
	}

	UE_LOG(LogGCFSystem, Log, TEXT("GCFInputBindingManager: Conditions met. Processing %d bindings."), PendingBindings.Num());


	for (auto It = PendingBindings.CreateIterator(); It; ++It) {
		const FGCFPendingBindingKey& Key = It.Key();
		const FGCFInputBindNativeDelegate& Delegate = It.Value();

		UObject* Binder = Key.Binder.Get();
		if (!Binder) {
			// Clean up stale requests from destroyed objects
			It.RemoveCurrent();
			continue;
		}

		const bool bIsBinderPawnDependent = IsBinderPawnDependent(Binder);

		// [Check] Prevent redundant binding execution for Controller-persistent bindings.
		// Since persistent bindings remain in the pending list, we must skip them if they are already active.
		bool bAlreadyActive = false;
		for (const auto& Group : ActiveBindingGroups) {
			if (Group.Key == Key)
			{
				bAlreadyActive = true;
				break;
			}
		}

		// Skip if active, BUT always re-process if it's Pawn-dependent (as it's a new instance/lifecycle).
		if (bAlreadyActive && !bIsBinderPawnDependent) {
			continue;
		}


		// Execute the binding
		ExecuteInputBinding(
			Key,
			Delegate,
			InputComp,
			bIsBinderPawnDependent ? PawnProvider : PlayerProvider
		);

		// [Logic] List Management
		// Pawn-dependent bindings: Remove from pending. They will re-register when the next Pawn spawns.
		// Controller-persistent bindings: Keep in pending. They persist across pawn swaps and need to be protected from removal.
		if (bIsBinderPawnDependent) {
			It.RemoveCurrent();
		}
	}
}


void UGCFInputBindingManagerComponent::ApplyPawnMappingContexts(AActor* PawnActor)
{
	if (!PawnActor) {
		return;
	}

	APlayerController* PC = GetController<APlayerController>();
	UEnhancedInputLocalPlayerSubsystem* Subsystem = PC && PC->GetLocalPlayer() ?
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()) : nullptr;

	if (!Subsystem) {
		return;
	}

	if (TScriptInterface<IGCFPawnDataProvider> DataProvider = UGCFActorFunctionLibrary::ResolvePawnDataProvider(PawnActor)) {
		if (const UGCFPawnData* PawnData = DataProvider->GetPawnData<UGCFPawnData>()) {

			int32 AddedCount = 0;

			// Add all IMCs defined in the PawnData
			for (const FGCFInputMappingContextInfo& BindInfo : PawnData->DefaultMappingContexts) {
				if (BindInfo.InputMapping) {
					// Add the IMC with the specified priority
					Subsystem->AddMappingContext(BindInfo.InputMapping, BindInfo.Priority);

					// Record the IMC for future cleanup
					AppliedPawnIMCs.Add(BindInfo.InputMapping);
					AddedCount++;
				}
			}

			if (AddedCount > 0) {
				UE_LOG(LogGCFSystem, Log, TEXT("GCFInputBindingManager: Applied %d new Mapping Contexts for Pawn [%s]. Total active Pawn IMCs: %d."),
					   AddedCount, *PawnActor->GetName(), AppliedPawnIMCs.Num());
			}
		}
	}
}


void UGCFInputBindingManagerComponent::ClearPawnMappingContexts()
{
	if (AppliedPawnIMCs.IsEmpty()) {
		return;
	}

	APlayerController* PC = GetController<APlayerController>();
	UEnhancedInputLocalPlayerSubsystem* Subsystem = PC && PC->GetLocalPlayer() ?
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()) : nullptr;

	if (Subsystem) {
		// Remove all previously recorded IMCs from the subsystem
		for (const UInputMappingContext* IMC : AppliedPawnIMCs) {
			if (IMC) {
				Subsystem->RemoveMappingContext(IMC);
			}
		}
	}

	const int32 ClearedCount = AppliedPawnIMCs.Num();

	// Reset the tracking array
	AppliedPawnIMCs.Empty();

	UE_LOG(LogGCFSystem, Log, TEXT("GCFInputBindingManager: Cleared %d Pawn-dependent Mapping Contexts."), ClearedCount);
}


bool UGCFInputBindingManagerComponent::IsBinderPawnDependent(UObject* Binder) const
{
	if (Binder) {
		// If the binder is a Pawn itself or a component attached to a Pawn, it is dependent.
		if (Binder->IsA<APawn>()) {
			return true;
		}
		if (Binder->GetTypedOuter<APawn>() != nullptr) {
			return true;
		}
	}
	return false;
}


void UGCFInputBindingManagerComponent::EvaluateBindingConditions()
{
	if (bInputEnabled) {
		ProcessPendingBindings();
		SendDebugInfo();
	}
}


void UGCFInputBindingManagerComponent::ExecuteInputBinding(const FGCFPendingBindingKey& Key, const FGCFInputBindNativeDelegate& Delegate, UGCFInputComponent* InputComp, TScriptInterface<IGCFInputConfigProvider> Provider)
{
	// [Safety & Idempotency Check]
	// Check for existing bindings with the same Key/Binder.
	// This prevents duplicate bindings if logic triggers multiple times.
	for (int32 i = ActiveBindingGroups.Num() - 1; i >= 0; --i) {
		if (ActiveBindingGroups[i].Key == Key) {

			// Only attempt to remove binding if the InputComponent is the SAME instance.
			// If InputComponent has changed (e.g., Seamless Travel), the old bindings are implicitly invalid.
			UInputComponent* OldInputComp = ActiveBindingGroups[i].BoundInputComponent.Get();
			if (OldInputComp && OldInputComp == InputComp) {
				for (auto& Receipt : ActiveBindingGroups[i].Receipts) {
					InputComp->RemoveBinding(*Receipt.BindingPtr);
				}
				ActiveBindingGroups.RemoveAt(i);
			}
		}
	}

	// Execute Delegate
	TArray<FGCFBindingReceipt> Receipts = Delegate.Execute(InputComp, Provider);
	if (Receipts.IsEmpty()) {
		return;
	}

	// Register to active groups
	FGCFInputBindingGroup& Group = ActiveBindingGroups.AddDefaulted_GetRef();
	Group.Key = Key;
	Group.Receipts = Receipts;
	Group.BoundInputComponent = InputComp; // Track the physical component for safety
	Group.bIsPawnDependent = IsBinderPawnDependent(Key.Binder.Get());

#if !UE_BUILD_SHIPPING
	for (const auto& Receipt : Receipts) {
		Group.DebugBindingInfos.Add(FString::Printf(TEXT("%s : %s (%s)"),
									*Receipt.AssociatedTag.ToString(),
									Receipt.BindingPtr ? *Receipt.BindingPtr->GetAction()->GetName() : TEXT("None"),
									*UGCFDebugFunctionLibrary::GetEnumName(Receipt.TriggerEvent)));
	}
#endif
}


void UGCFInputBindingManagerComponent::ClearBindingsOnContextChange()
{
	APlayerController* PC = GetController<APlayerController>();
	UGCFInputComponent* GCFIC = PC ? Cast<UGCFInputComponent>(PC->InputComponent) : nullptr;

	// Iterate backwards to remove
	for (int32 i = ActiveBindingGroups.Num() - 1; i >= 0; --i) {
		FGCFInputBindingGroup& Group = ActiveBindingGroups[i];

		// [Logic] Filter Dependencies
		// Controller-persistent bindings are preserved (e.g. Menu, Chat).
		// Only clear bindings tied to the specific Pawn instance.
		if (!Group.bIsPawnDependent) {
			continue;
		}

		// Physically remove from Enhanced Input system
		if (GCFIC) {
			for (const auto& Receipt : Group.Receipts) {
				if (Receipt.BindingPtr) {
					GCFIC->RemoveBinding(*Receipt.BindingPtr);
				}
			}
		}

		// Remove from management list
		ActiveBindingGroups.RemoveAt(i);
	}

	UE_LOG(LogGCFSystem, Log, TEXT("GCFInputBindingManager: Cleared Pawn-dependent bindings."));
}


void UGCFInputBindingManagerComponent::SendDebugInfo()
{
#if !UE_BUILD_SHIPPING
	FGCFDebugInputSnapshot Payload;
	
	for (const FGCFInputBindingGroup& Group : ActiveBindingGroups) {
		FGCFDebugInputGroup GroupInfo;
		FString BinderName = Group.Key.Binder.IsValid() ? Group.Key.Binder->GetName() : TEXT("DEAD_OBJECT");
		FString Dependency = Group.bIsPawnDependent ? TEXT("[Pawn Dependent]") : TEXT("[Controller Persistent]");
		GroupInfo.GroupName = FString::Printf(TEXT("Binder: %s %s"), *BinderName, *Dependency);
		GroupInfo.ActiveBindings = Group.DebugBindingInfos;
		Payload.Groups.Add(GroupInfo);
	}

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(GCFGameplayTags::Message_Debug_Input, Payload);
#endif
}


void UGCFInputBindingManagerComponent::DumpInputBindings()
{
#if !UE_BUILD_SHIPPING
	UE_LOG(LogGCFSystem, Display, TEXT("=== GCF Input Binding Manager Dump ==="));

	// Active Bindings
	UE_LOG(LogGCFSystem, Display, TEXT("--- Active Binding Groups (%d) ---"), ActiveBindingGroups.Num());
	for (const FGCFInputBindingGroup& Group : ActiveBindingGroups) {
		FString BinderName = Group.Key.Binder.IsValid() ? Group.Key.Binder->GetName() : TEXT("DEAD_OBJECT");
		FString Dependency = Group.bIsPawnDependent ? TEXT("[Pawn Dependent]") : TEXT("[Controller Persistent]");

		UE_LOG(LogGCFSystem, Display, TEXT(" > Binder: %s %s"), *BinderName, *Dependency);

		// Output debug strings saved during ExecuteInputBinding
		for (const FString& Info : Group.DebugBindingInfos) {
			UE_LOG(LogGCFSystem, Display, TEXT("    - %s"), *Info);
		}
	}

	// Pending Bindings
	UE_LOG(LogGCFSystem, Display, TEXT("--- Pending Bindings (%d) ---"), PendingBindings.Num());
	for (auto It = PendingBindings.CreateIterator(); It; ++It) {
		const FGCFPendingBindingKey& BindingKey = It.Key();
		const FGCFInputBindNativeDelegate& Delegate = It.Value();

		// 1. Context Status
		FString ContextName = BindingKey.Binder.IsValid()
			? BindingKey.Binder->GetName()
			// Use GetEvenIfUnreachable only for logging dead objects safely
			: FString::Printf(TEXT("EXPIRED (Previously: %s)"), *BindingKey.Binder.GetEvenIfUnreachable()->GetName());

		// 2. Function/Key Name
		FString FunctionName = BindingKey.KeyName.ToString();

		// 3. Dependency Scope
		const bool bIsPawnDep = IsBinderPawnDependent(BindingKey.Binder.Get());
		FString Scope = bIsPawnDep ? TEXT("[Pawn-Dependent]") : TEXT("[Controller-Persistent]");

		// Log: [Scope] ContextName :: FunctionName
		UE_LOG(LogGCFSystem, Display, TEXT(" - %-25s %s :: %s"), *Scope, *ContextName, *FunctionName);
	}

	UE_LOG(LogGCFSystem, Display, TEXT("======================================="));
#endif
}