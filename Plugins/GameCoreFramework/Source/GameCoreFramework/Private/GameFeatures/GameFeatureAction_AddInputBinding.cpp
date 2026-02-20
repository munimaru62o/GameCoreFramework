// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#include "Gamefeatures/GameFeatureAction_AddInputBinding.h"
#include "GameFeatures/GameFeatureAction_WorldActionBase.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Input/GCFInputConfig.h"
#include "Input/GCFPlayerInputBridgeComponent.h"
#include "Input/GCFInputFunctionLibrary.h"
#include "GCFShared.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddInputBinding)

#define LOCTEXT_NAMESPACE "GameFeatures"

//////////////////////////////////////////////////////////////////////
// UGameFeatureAction_AddInputBinding

void UGameFeatureAction_AddInputBinding::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);
	if (!ensure(ActiveData.ExtensionRequestHandles.IsEmpty()) ||
		!ensure(ActiveData.ControllersAddedTo.IsEmpty()))
	{
		Reset(ActiveData);
	}
	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddInputBinding::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	FPerContextData* ActiveData = ContextData.Find(Context);

	if (ensure(ActiveData))
	{
		Reset(*ActiveData);
	}
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddInputBinding::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	int32 Index = 0;

	for (const TSoftObjectPtr<const UGCFInputConfig>& Entry : InputConfigs)
	{
		if (Entry.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("NullInputConfig", "Null InputConfig at index {0}."), Index));
		}
		++Index;
	}

	return Result;
}
#endif

void UGameFeatureAction_AddInputBinding::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate =
				UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandlePawnExtension, ChangeContext);
			TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle =
				ComponentManager->AddExtensionHandler(APawn::StaticClass(), AddAbilitiesDelegate);

			ActiveData.ExtensionRequestHandles.Add(ExtensionRequestHandle);
		}
	}
}

void UGameFeatureAction_AddInputBinding::Reset(FPerContextData& ActiveData)
{
	ActiveData.ExtensionRequestHandles.Empty();

	while (!ActiveData.ControllersAddedTo.IsEmpty())
	{
		TWeakObjectPtr<APlayerController> ControllerPtr = ActiveData.ControllersAddedTo.Top();
		if (ControllerPtr.IsValid())
		{
			RemoveInputForPlayer(ControllerPtr.Get(), ActiveData);
		}
		else
		{
			ActiveData.ControllersAddedTo.Pop();
		}
	}
}

void UGameFeatureAction_AddInputBinding::HandlePawnExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	APawn* Pawn = Cast<APawn>(Actor);
	if (!Pawn) {
		return;
	}

	APlayerController* Controller = Cast<APlayerController>(Pawn->GetController());
	if (!Controller) {
		return;
	}

	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded || EventName == GCF::Names::Event_Pawn_OnPossessed) {
		AddInputForPlayer(Controller, ActiveData);
	} else if (
		EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved || EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved) {
		RemoveInputForPlayer(Controller, ActiveData);
	}
}

void UGameFeatureAction_AddInputBinding::AddInputForPlayer(APlayerController* Controller, FPerContextData& ActiveData)
{
	if (!Controller || !Controller->IsLocalController()) {
		return;
	}

	if (ActiveData.ControllersAddedTo.Contains(Controller)) {
		return;
	}

	// Register Input Config (Connecting to the InputBindingManager)
	// This uses the Bridge Component we created earlier.
	if (TScriptInterface<IGCFInputConfigProvider> Provider = UGCFInputFunctionLibrary::ResolveInputConfigProvider(Controller, EGCFInputSourceType::Player)) {
		for (const TSoftObjectPtr<const UGCFInputConfig>& Entry : InputConfigs) {
			if (const UGCFInputConfig* Config = Entry.Get()) {
				Provider->AddInputConfig(Config);
			}
		}

		ActiveData.ControllersAddedTo.AddUnique(Controller);
	}
}

void UGameFeatureAction_AddInputBinding::RemoveInputForPlayer(APlayerController* Controller, FPerContextData& ActiveData)
{
	if (!Controller) {
		return;
	}

	if (UGCFPlayerInputBridgeComponent* Bridge = Controller->FindComponentByClass<UGCFPlayerInputBridgeComponent>()) {
		for (const TSoftObjectPtr<const UGCFInputConfig>& Entry : InputConfigs) {
			if (const UGCFInputConfig* Config = Entry.Get()) {
				Bridge->RemoveInputConfig(Config);
			}
		}
	}

	ActiveData.ControllersAddedTo.Remove(Controller);
}

#undef LOCTEXT_NAMESPACE

