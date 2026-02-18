// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/GCFGameSession.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFGameSession)


AGCFGameSession::AGCFGameSession(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool AGCFGameSession::ProcessAutoLogin()
{
	// This is actually handled in GCFGameMode::TryDedicatedServerLogin
	return true;
}

void AGCFGameSession::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

void AGCFGameSession::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
}

