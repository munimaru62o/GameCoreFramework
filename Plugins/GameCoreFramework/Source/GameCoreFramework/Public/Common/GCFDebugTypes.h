// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#include "CoreMinimal.h"
#include "GCFDebugTypes.generated.h"

class UObject;

UENUM(BlueprintType)
enum class EGCFDebugLogVerbosity : uint8
{
    Info,
    Success, // Used for green text formatting
    Warning, // Used for yellow text formatting
    Error    // Used for gray text formatting (e.g., No Target, Failures)
};

UENUM(BlueprintType)
enum class EGCFDebugStateCategory : uint8
{
    None,
    ReadyStatePlayer,
    ReadyStatePawn,
    Possession,
    InputContext,
};

USTRUCT(BlueprintType)
struct FGCFDebugLogEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EGCFDebugLogVerbosity LogType = EGCFDebugLogVerbosity::Info;

    UPROPERTY(BlueprintReadWrite)
    FString Message;
};

USTRUCT(BlueprintType)
struct FGCFDebugStateEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EGCFDebugStateCategory Category = EGCFDebugStateCategory::None;

    // The item label to update (e.g., "Possession", "InputContext", "ReadyState")
    UPROPERTY(BlueprintReadWrite)
    FString Label;

    // The content value to display (e.g., "HumanPawn", "IMC_Vehicle")
    UPROPERTY(BlueprintReadWrite)
    FString Value;

    // Optional display color based on the current state
    UPROPERTY(BlueprintReadWrite)
    FLinearColor DisplayColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FGCFDebugInputGroup
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString GroupName;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> ActiveBindings;
};

USTRUCT(BlueprintType)
struct FGCFDebugInputSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TArray<FGCFDebugInputGroup> Groups;
};