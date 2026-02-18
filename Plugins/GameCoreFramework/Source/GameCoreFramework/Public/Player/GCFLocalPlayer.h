// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonLocalPlayer.h"
#include "Actor/GCFTeamAgentInterface.h"

#include "GCFLocalPlayer.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

struct FGenericTeamId;

class APlayerController;
class UInputMappingContext;
class UGCFSettingsLocal;
class UGCFSettingsShared;
class UObject;
class UWorld;
struct FFrame;
struct FSwapAudioOutputResult;

/**
 * UGCFLocalPlayer
 */
UCLASS(MinimalAPI)
class UGCFLocalPlayer : public UCommonLocalPlayer, public IGCFTeamAgentInterface
{
	GENERATED_BODY()

public:

	UE_API UGCFLocalPlayer();

	//~UObject interface
	UE_API virtual void PostInitProperties() override;
	//~End of UObject interface

	//~UPlayer interface
	UE_API virtual void SwitchController(class APlayerController* PC) override;
	//~End of UPlayer interface

	//~ULocalPlayer interface
	UE_API virtual bool SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld) override;
	UE_API virtual void InitOnlineSession() override;
	//~End of ULocalPlayer interface

	//~IGCFTeamAgentInterface interface
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnGCFTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IGCFTeamAgentInterface interface

	/** Gets the local settings for this player, this is read from config files at process startup and is always valid */
	UFUNCTION()
	UE_API UGCFSettingsLocal* GetLocalSettings() const;

	/** Gets the shared setting for this player, this is read using the save game system so may not be correct until after user login */
	UFUNCTION()
	UE_API UGCFSettingsShared* GetSharedSettings() const;

	/** Starts an async request to load the shared settings, this will call OnSharedSettingsLoaded after loading or creating new ones */
	UE_API void LoadSharedSettingsFromDisk(bool bForceLoad = false);

protected:
	UE_API void OnSharedSettingsLoaded(UGCFSettingsShared* LoadedOrCreatedSettings);

	UE_API void OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId);

	UFUNCTION()
	UE_API void OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult);

	UE_API void OnPlayerControllerChanged(APlayerController* NewController);

	UFUNCTION()
	UE_API void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

private:
	UPROPERTY(Transient)
	mutable TObjectPtr<UGCFSettingsShared> SharedSettings;

	FUniqueNetIdRepl NetIdForSharedSettings;

	UPROPERTY(Transient)
	mutable TObjectPtr<const UInputMappingContext> InputMappingContext;

	UPROPERTY()
	FOnGCFTeamIndexChangedDelegate OnTeamChangedDelegate;

	UPROPERTY()
	TWeakObjectPtr<APlayerController> LastBoundPC;
};

#undef UE_API
