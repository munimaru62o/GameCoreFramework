// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameSession.h"
#include "GCFGameSession.generated.h"

class UObject;


UCLASS(Config = Game)
class AGCFGameSession : public AGameSession
{
	GENERATED_BODY()

public:

	AGCFGameSession(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	/** Override to disable the default behavior */
	virtual bool ProcessAutoLogin() override;

	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
};
