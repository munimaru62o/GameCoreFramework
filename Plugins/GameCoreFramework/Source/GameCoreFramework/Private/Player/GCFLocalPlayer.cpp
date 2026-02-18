// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/GCFLocalPlayer.h"

#include "AudioMixerBlueprintLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Player/GCFSettingsLocal.h"
#include "Player/GCFSettingsShared.h"
#include "CommonUserSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFLocalPlayer)

class UObject;

UGCFLocalPlayer::UGCFLocalPlayer()
{}

void UGCFLocalPlayer::PostInitProperties()
{
	Super::PostInitProperties();

	if (UGCFSettingsLocal* LocalSettings = GetLocalSettings()) {
		//LocalSettings->OnAudioOutputDeviceChanged.AddUObject(this, &UGCFLocalPlayer::OnAudioOutputDeviceChanged);
	}
}

void UGCFLocalPlayer::SwitchController(class APlayerController* PC)
{
	Super::SwitchController(PC);

	OnPlayerControllerChanged(PlayerController);
}

bool UGCFLocalPlayer::SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld)
{
	const bool bResult = Super::SpawnPlayActor(URL, OutError, InWorld);

	OnPlayerControllerChanged(PlayerController);

	return bResult;
}

void UGCFLocalPlayer::InitOnlineSession()
{
	OnPlayerControllerChanged(PlayerController);

	Super::InitOnlineSession();
}

void UGCFLocalPlayer::OnPlayerControllerChanged(APlayerController* NewController)
{
	// Stop listening for changes from the old controller
	FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
	if (IGCFTeamAgentInterface* ControllerAsTeamProvider = Cast<IGCFTeamAgentInterface>(LastBoundPC.Get())) {
		OldTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}

	// Grab the current team ID and listen for future changes
	FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
	if (IGCFTeamAgentInterface* ControllerAsTeamProvider = Cast<IGCFTeamAgentInterface>(NewController)) {
		NewTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
		LastBoundPC = NewController;
	}

	ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
}

void UGCFLocalPlayer::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	// Do nothing, we merely observe the team of our associated player controller
}

FGenericTeamId UGCFLocalPlayer::GetGenericTeamId() const
{
	if (IGCFTeamAgentInterface* ControllerAsTeamProvider = Cast<IGCFTeamAgentInterface>(PlayerController)) {
		return ControllerAsTeamProvider->GetGenericTeamId();
	} else {
		return FGenericTeamId::NoTeam;
	}
}

FOnGCFTeamIndexChangedDelegate* UGCFLocalPlayer::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}

UGCFSettingsLocal* UGCFLocalPlayer::GetLocalSettings() const
{
	return UGCFSettingsLocal::Get();
}

UGCFSettingsShared* UGCFLocalPlayer::GetSharedSettings() const
{
	if (!SharedSettings) {
		// On PC it's okay to use the sync load because it only checks the disk
		// This could use a platform tag to check for proper save support instead
		bool bCanLoadBeforeLogin = PLATFORM_DESKTOP;

		if (bCanLoadBeforeLogin) {
			SharedSettings = UGCFSettingsShared::LoadOrCreateSettings(this);
		} else {
			// We need to wait for user login to get the real settings so return temp ones
			SharedSettings = UGCFSettingsShared::CreateTemporarySettings(this);
		}
	}

	return SharedSettings;
}

void UGCFLocalPlayer::LoadSharedSettingsFromDisk(bool bForceLoad)
{
	FUniqueNetIdRepl CurrentNetId = GetCachedUniqueNetId();
	if (!bForceLoad && SharedSettings && CurrentNetId == NetIdForSharedSettings) {
		// Already loaded once, don't reload
		return;
	}

	ensure(UGCFSettingsShared::AsyncLoadOrCreateSettings(this, UGCFSettingsShared::FOnSettingsLoadedEvent::CreateUObject(this, &UGCFLocalPlayer::OnSharedSettingsLoaded)));
}

void UGCFLocalPlayer::OnSharedSettingsLoaded(UGCFSettingsShared* LoadedOrCreatedSettings)
{
	// The settings are applied before it gets here
	if (ensure(LoadedOrCreatedSettings)) {
		// This will replace the temporary or previously loaded object which will GC out normally
		SharedSettings = LoadedOrCreatedSettings;

		NetIdForSharedSettings = GetCachedUniqueNetId();
	}
}

void UGCFLocalPlayer::OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId)
{
	FOnCompletedDeviceSwap DevicesSwappedCallback;
	DevicesSwappedCallback.BindUFunction(this, FName("OnCompletedAudioDeviceSwap"));
	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(GetWorld(), InAudioOutputDeviceId, DevicesSwappedCallback);
}

void UGCFLocalPlayer::OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult)
{
	if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Failure) {
	}
}

void UGCFLocalPlayer::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	ConditionalBroadcastTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));
}

