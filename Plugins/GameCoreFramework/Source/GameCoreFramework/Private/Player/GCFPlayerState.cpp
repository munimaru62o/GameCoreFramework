// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 62o. All rights reserved.

#include "Player/GCFPlayerState.h"

#include "GCFShared.h"
#include "AbilitySystem/AttributeSet/GCFCombatAttributeSet.h"
#include "AbilitySystem/AttributeSet/GCFHealthAttributeSet.h"
#include "AbilitySystem/GCFAbilitySet.h"
#include "AbilitySystem/GCFAbilitySystemComponent.h"
#include "Actor/Data/GCFPawnData.h"
#include "System/Lifecycle/GCFPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Experience/GCFExperienceManagerComponent.h"
//@TODO: Would like to isolate this a bit better to get the pawn data in here without this having to know about other stuff
#include "System/GCFGameMode.h"
#include "Player/GCFPlayerController.h"
#include "System/Lifecycle/GCFPlayerReadyStateComponent.h"
#include "System/Lifecycle/GCFPlayerExtensionComponent.h"
#include "Messages/GCFVerbMessage.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFPlayerState)

class AController;
class APlayerState;
class FLifetimeProperty;


AGCFPlayerState::AGCFPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MyPlayerConnectionType(EGCFPlayerConnectionType::Player)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UGCFAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	PlayerExtensionComponent = ObjectInitializer.CreateDefaultSubobject<UGCFPlayerExtensionComponent>(this, TEXT("PlayerExtensionComponent"));
	PlayerReadyStateComponent = ObjectInitializer.CreateDefaultSubobject<UGCFPlayerReadyStateComponent>(this, TEXT("PlayerReadyStateComponent"));

	// AbilitySystemComponent needs to be updated at a high frequency.
	SetNetUpdateFrequency(100.0f);

	MyTeamID = FGenericTeamId::NoTeam;
	MySquadID = INDEX_NONE;
}

void AGCFPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AGCFPlayerState::Reset()
{
	Super::Reset();
}

void AGCFPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);

	if (UGCFPawnExtensionComponent* PawnExtComp = UGCFPawnExtensionComponent::FindGCFPawnExtensionComponent(GetPawn()))
	{
		PawnExtComp->CheckDefaultInitialization();
	}
	if (PlayerExtensionComponent) {
		PlayerExtensionComponent->CheckDefaultInitialization();
	}
}

void AGCFPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	//@TODO: Copy stats
}

void AGCFPlayerState::OnDeactivated()
{
	bool bDestroyDeactivatedPlayerState = false;

	switch (GetPlayerConnectionType())
	{
		case EGCFPlayerConnectionType::Player:
		case EGCFPlayerConnectionType::InactivePlayer:
			//@TODO: Ask the experience if we should destroy disconnecting players immediately or leave them around
			// (e.g., for long running servers where they might build up if lots of players cycle through)
			bDestroyDeactivatedPlayerState = true;
			break;
		default:
			bDestroyDeactivatedPlayerState = true;
			break;
	}
	
	SetPlayerConnectionType(EGCFPlayerConnectionType::InactivePlayer);

	if (bDestroyDeactivatedPlayerState)
	{
		Destroy();
	}
}

void AGCFPlayerState::OnReactivated()
{
	if (GetPlayerConnectionType() == EGCFPlayerConnectionType::InactivePlayer)
	{
		SetPlayerConnectionType(EGCFPlayerConnectionType::Player);
	}
}

void AGCFPlayerState::OnExperienceLoaded(const UGCFExperienceDefinition* /*CurrentExperience*/)
{
	if (AGCFGameMode* GCFGameMode = GetWorld()->GetAuthGameMode<AGCFGameMode>())
	{
		if (const UGCFPawnData* NewPawnData = GCFGameMode->GetPawnDataForController(GetOwningController()))
		{
			SetPawnData(NewPawnData);
		}
		else
		{
			UE_LOG(LogGCFPlayer, Error, TEXT("AGCFPlayerState::OnExperienceLoaded(): Unable to find PawnData to initialize player state [%s]!"), *GetNameSafe(this));
		}
	}
}

void AGCFPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyPlayerConnectionType, SharedParams)
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyTeamID, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MySquadID, SharedParams);

	SharedParams.Condition = ELifetimeCondition::COND_SkipOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedViewRotation, SharedParams);

	DOREPLIFETIME(ThisClass, StatTags);	
}

FRotator AGCFPlayerState::GetReplicatedViewRotation() const
{
	// Could replace this with custom replication
	return ReplicatedViewRotation;
}

void AGCFPlayerState::SetReplicatedViewRotation(const FRotator& NewRotation)
{
	if (NewRotation != ReplicatedViewRotation)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ReplicatedViewRotation, this);
		ReplicatedViewRotation = NewRotation;
	}
}


UAbilitySystemComponent* AGCFPlayerState::GetAbilitySystemComponent() const
{
	return GetGCFAbilitySystemComponent();
}

void AGCFPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();


	UWorld* World = GetWorld();
	if (World && World->IsGameWorld() && World->GetNetMode() != NM_Client)
	{
		AGameStateBase* GameState = GetWorld()->GetGameState();
		check(GameState);
		UGCFExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UGCFExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnGCFExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}
}

void AGCFPlayerState::SetPawnData(const UGCFPawnData* InPawnData)
{
	check(InPawnData);

	if (GetLocalRole() != ROLE_Authority) {
		return;
	}

	if (PawnData) {
		UE_LOG(LogGCFPlayer, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	ForceNetUpdate();
}


void AGCFPlayerState::OnRep_PawnData()
{
}


void AGCFPlayerState::OnRep_Owner()
{
	if (PlayerExtensionComponent) {
		PlayerExtensionComponent->NotifyOwningControllerChanged();
	}
}


void AGCFPlayerState::SetPlayerConnectionType(EGCFPlayerConnectionType NewType)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyPlayerConnectionType, this);
	MyPlayerConnectionType = NewType;
}

void AGCFPlayerState::SetSquadID(int32 NewSquadId)
{
	if (HasAuthority())
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MySquadID, this);

		MySquadID = NewSquadId;
	}
}

void AGCFPlayerState::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (HasAuthority())
	{
		const FGenericTeamId OldTeamID = MyTeamID;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyTeamID, this);
		MyTeamID = NewTeamID;
		ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
	}
	else
	{
		UE_LOG(LogGCFPlayer, Error, TEXT("Cannot set team for %s on non-authority"), *GetPathName(this));
	}
}

FGenericTeamId AGCFPlayerState::GetGenericTeamId() const
{
	return MyTeamID;
}

FOnGCFTeamIndexChangedDelegate* AGCFPlayerState::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

void AGCFPlayerState::OnRep_MyTeamID(FGenericTeamId OldTeamID)
{
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void AGCFPlayerState::OnRep_MySquadID()
{
	//@TODO: Let the squad subsystem know (once that exists)
}

void AGCFPlayerState::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void AGCFPlayerState::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

int32 AGCFPlayerState::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

bool AGCFPlayerState::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

void AGCFPlayerState::ClientBroadcastMessage_Implementation(const FGCFVerbMessage Message)
{
	// This check is needed to prevent running the action when in standalone mode
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
	}
}

