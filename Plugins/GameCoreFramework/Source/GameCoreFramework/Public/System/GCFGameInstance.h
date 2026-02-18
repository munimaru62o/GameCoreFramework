// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonGameInstance.h"
#include "GCFGameInstance.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class AGCFPlayerController;
class UObject;
class UGCFUserFacingExperienceDefinition;

UCLASS(MinimalAPI, Config = Game)
class UGCFGameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

public:

	UE_API UGCFGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API AGCFPlayerController* GetPrimaryPlayerController() const;
	
	UE_API virtual bool CanJoinRequestedSession() const override;
	UE_API virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext) override;

	UE_API virtual void ReceivedNetworkEncryptionToken(const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate) override;
	UE_API virtual void ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate) override;

	//UFUNCTION(BlueprintCallable, Category = "GCF|Experience")
	//UE_API virtual void RequestExperienceLoad(const UGCFUserFacingExperienceDefinition* UserFacingDef);


protected:

	UE_API virtual void Init() override;
	UE_API virtual void Shutdown() override;

	UE_API void OnPreClientTravelToSession(FString& URL);

	/** A hard-coded encryption key used to try out the encryption code. This is NOT SECURE, do not use this technique in production! */
	TArray<uint8> DebugTestEncryptionKey;
};

#undef UE_API
