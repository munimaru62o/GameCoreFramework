// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonGameInstance.h"
#include "GCFGameInstance.generated.h"

class AGCFPlayerController;
class UObject;
class UGCFUserFacingExperienceDefinition;

UCLASS(Config = Game)
class GAMECOREFRAMEWORK_API UGCFGameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

public:

	UGCFGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	AGCFPlayerController* GetPrimaryPlayerController() const;
	
	virtual bool CanJoinRequestedSession() const override;
	virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext) override;

	virtual void ReceivedNetworkEncryptionToken(const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate) override;
	virtual void ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate) override;

	//UFUNCTION(BlueprintCallable, Category = "GCF|Experience")
	//virtual void RequestExperienceLoad(const UGCFUserFacingExperienceDefinition* UserFacingDef);


protected:

	virtual void Init() override;
	virtual void Shutdown() override;

	void OnPreClientTravelToSession(FString& URL);

	/** A hard-coded encryption key used to try out the encryption code. This is NOT SECURE, do not use this technique in production! */
	TArray<uint8> DebugTestEncryptionKey;
};
