// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Input/GCFAbilityInputRouterComponent.h"

#include "GCFShared.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/GCFAbilitySystemComponent.h"
#include "System/Binder/GCFControllerPossessionBinder.h"
#include "System/Binder/GCFPlayerReadyStateBinder.h"
#include "Components/GameFrameworkComponentManager.h"

#include "AbilitySystem/GCFAbilitySystemFunctionLibrary.h" 
#include "Misc/EnumClassFlags.h"

#include "System/GCFDebugFunctionLibrary.h"


UGCFAbilityInputRouterComponent::UGCFAbilityInputRouterComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bWantsInitializeComponent = true;
}


void UGCFAbilityInputRouterComponent::InitializeComponent()
{
	if (AController* Controller = GetController<AController>()) {
		// Only active for Local Players (Input Source)
		if (!Controller->IsLocalController()) {
			Deactivate();
			return;
		}

		if (UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(GetOwner())) {
			// Observe Possession changes to update the "Body" ASC reference
			BinderList.Emplace(FGCFControllerPossessionBinder::CreateBinder(GFCM, Controller, FGCFBooleanStateSignature::CreateUObject(this, &ThisClass::HandlePossessedPawnChanged)));

			// Observe Player Readiness to update the "Soul" ASC reference
			BinderList.Emplace(FGCFPlayerReadyStateBinder::CreateBinder(GFCM, Controller, FGCFOnPlayerReadyStateChangedNative::FDelegate::CreateUObject(this, &ThisClass::HandlePlayerReadyStateChanged)));
		}
	}
}


void UGCFAbilityInputRouterComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UGCFAbilityInputRouterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}


void UGCFAbilityInputRouterComponent::HandlePossessedPawnChanged(AActor* Actor, bool bPossessed)
{
	if (bPossessed) {
		if (APawn* Pawn = Cast<APawn>(Actor)) {
			CurrentPawnASC = UGCFAbilitySystemFunctionLibrary::GetPawnAbilitySystemComponent<UGCFAbilitySystemComponent>(Pawn);
		}
	} else {
		CurrentPawnASC.Reset();
	}
}


void UGCFAbilityInputRouterComponent::HandlePlayerReadyStateChanged(const FGCFPlayerReadyStateSnapshot& Snapshot)
{
	static const EGCFPlayerReadyState Required = EGCFPlayerReadyState::GamePlay;
	if (GCF::Bitmask::HasFlagsChanged(Snapshot.State, CachedPlayerReadyState, Required)) {
		if (GCF::Bitmask::AreFlagsSet(Snapshot.State, Required)) {
			PlayerStateASC = UGCFAbilitySystemFunctionLibrary::GetPlayerStateAbilitySystemComponent<UGCFAbilitySystemComponent>(Snapshot.PlayerState.Get());
		}
	}
	CachedPlayerReadyState = Snapshot.State;
}


void UGCFAbilityInputRouterComponent::RouteInputTag(const FGameplayTag& InputTag, bool bPressed)
{
	// Determine the routing destination based on the tag namespace.
	const bool bIsPlayerTag = InputTag.MatchesTag(GCFGameplayTags::InputTag_Ability_Player);
	const bool bIsPawnTag = InputTag.MatchesTag(GCFGameplayTags::InputTag_Ability_Pawn);

	if (!bIsPlayerTag && !bIsPawnTag) {
		UE_LOG(LogGCFSystem, Warning, TEXT(
			"GCFAbilityInputRouter: Invalid input tag [%s]. "
			"Expected Ability.Input.Player.* or Ability.Input.Pawn.*"
		), *InputTag.ToString());
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = InputTag;
	EventData.Instigator = GetOwner();
	EventData.Target = GetOwner();

	// Route to "Soul" (PlayerState)
	if (bIsPlayerTag && PlayerStateASC.Get()) {
		ExecRouteTag(PlayerStateASC.Get(), InputTag, bPressed);
	}

	// Route to "Body" (Pawn)
	if (bIsPawnTag && CurrentPawnASC.Get()) {
		ExecRouteTag(CurrentPawnASC.Get(), InputTag, bPressed);
	}
}


bool UGCFAbilityInputRouterComponent::HasMachingAbility(UAbilitySystemComponent* ASC, const FGameplayTag& InputTag) const
{
	if (!ASC) {
		return false;
	}

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities()) {
		if (Spec.GetDynamicSpecSourceTags().HasTag(InputTag)) {
			return true;
		}
	}
	return false;
}


void UGCFAbilityInputRouterComponent::ExecRouteTag(UGCFAbilitySystemComponent* ASC, const FGameplayTag& InputTag, bool bPressed) const
{
	if (!ASC) {
		return;
	}

	const bool bHasMachingAbility = HasMachingAbility(ASC, InputTag);
	if (bHasMachingAbility) {
		if (bPressed) {
			ASC->AbilityInputTagPressed(InputTag);
		} else {
			ASC->AbilityInputTagReleased(InputTag);
		}
	}

	if (bPressed) {
		if (bHasMachingAbility) {
			UGCFDebugFunctionLibrary::SendLogMessage(this, EGCFDebugLogVerbosity::Success, FString::Printf(TEXT("%s: Routed to %s"), *InputTag.GetTagName().ToString(), *ASC->GetOwner()->GetName()));
		} else {
			UGCFDebugFunctionLibrary::SendLogMessage(this, EGCFDebugLogVerbosity::Error, TEXT("No Target (Ability Not Found)"));
		}
	}
}