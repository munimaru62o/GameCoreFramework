// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Common/GCFLogChannels.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY(LogGCFCommon);
DEFINE_LOG_CATEGORY(LogGCFAbilitySystem);
DEFINE_LOG_CATEGORY(LogGCFCharacter);
DEFINE_LOG_CATEGORY(LogGCFPlayer);
DEFINE_LOG_CATEGORY(LogGCFSystem);
DEFINE_LOG_CATEGORY(LogGCFUI);
DEFINE_LOG_CATEGORY(LogGCFModular);

FString GetClientServerContextString(UObject* ContextObject)
{
	ENetRole Role = ROLE_None;

	if (AActor* Actor = Cast<AActor>(ContextObject)) {
		Role = Actor->GetLocalRole();
	} else if (UActorComponent* Component = Cast<UActorComponent>(ContextObject)) {
		Role = Component->GetOwnerRole();
	}

	if (Role != ROLE_None) {
		return (Role == ROLE_Authority) ? TEXT("Server") : TEXT("Client");
	} else {
#if WITH_EDITOR
		if (GIsEditor) {
			extern ENGINE_API FString GPlayInEditorContextString;
			return GPlayInEditorContextString;
		}
#endif
	}

	return TEXT("[]");
}
