// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "System/GCFDebugFunctionLibrary.h"

#include "GCFShared.h"
#include "Common/GCFTypes.h"
#include "System/Lifecycle/GCFPawnReadyStateComponent.h"
#include "System/Lifecycle/GCFPlayerReadyStateComponent.h"
#include "Player/GCFControllerPossessionComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"


FText UGCFDebugFunctionLibrary::FormatLogMessage(EGCFDebugLogVerbosity InLogType, const FString& InMessage)
{
    FString TagName;

    switch (InLogType) {
        case EGCFDebugLogVerbosity::Success:
            TagName = TEXT("Success");
            break;
        case EGCFDebugLogVerbosity::Warning:
            TagName = TEXT("Warning");
            break;
        case EGCFDebugLogVerbosity::Error:
            TagName = TEXT("Error");
            break;
        case EGCFDebugLogVerbosity::Info:
        default:
            TagName = TEXT("Default");
            break;
    }
    FString FormattedString = FString::Printf(TEXT("<%s>%s</>"), *TagName, *InMessage);
    return FText::FromString(FormattedString);
}


void UGCFDebugFunctionLibrary::SendLogMessage(const UObject* WorldContext, EGCFDebugLogVerbosity Type, const FString& Msg)
{
#if !UE_BUILD_SHIPPING
    FGCFDebugLogEntry Payload;
    Payload.LogType = Type;
    Payload.Message = Msg;

    UGameplayMessageSubsystem::Get(WorldContext).BroadcastMessage(GCFGameplayTags::Message_Debug_Log, Payload);
#endif
}


void UGCFDebugFunctionLibrary::SendStateMessage(const UObject* WorldContext, EGCFDebugStateCategory Category, const FString& NewValue, const FLinearColor& DisplayColor)
{
#if !UE_BUILD_SHIPPING
    FGCFDebugStateEntry Payload;
    Payload.Category = Category;
    Payload.Label = GetEnumName(Category);
    Payload.Value = NewValue;
    Payload.DisplayColor = DisplayColor;

    UGameplayMessageSubsystem::Get(WorldContext).BroadcastMessage(GCFGameplayTags::Message_Debug_State, Payload);
#endif
}


void UGCFDebugFunctionLibrary::SendPlayerStateBitMessage(const UObject* WorldContext, EGCFDebugStateCategory Category, EGCFPlayerReadyState State, const FLinearColor& DisplayColor)
{
#if !UE_BUILD_SHIPPING
    FGCFDebugStateEntry Payload;
    Payload.Category = Category;
    Payload.Label = GetEnumName(Category);
    Payload.Value = GetBitflagsString(State);
    Payload.DisplayColor = DisplayColor;

    UGameplayMessageSubsystem::Get(WorldContext).BroadcastMessage(GCFGameplayTags::Message_Debug_State, Payload);
#endif
}


void UGCFDebugFunctionLibrary::SendPawnStateBitMessage(const UObject* WorldContext, EGCFDebugStateCategory Category, EGCFPawnReadyState State, const FLinearColor& DisplayColor)
{
#if !UE_BUILD_SHIPPING
    FGCFDebugStateEntry Payload;
    Payload.Category = Category;
    Payload.Label = GetEnumName(Category);
    Payload.Value = GetBitflagsString(State);
    Payload.DisplayColor = DisplayColor;

    UGameplayMessageSubsystem::Get(WorldContext).BroadcastMessage(GCFGameplayTags::Message_Debug_State, Payload);
#endif
}