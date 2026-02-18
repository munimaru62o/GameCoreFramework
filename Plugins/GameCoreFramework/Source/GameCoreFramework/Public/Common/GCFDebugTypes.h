// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#include "CoreMinimal.h"
#include "GCFDebugTypes.generated.h"


class UObject;

UENUM(BlueprintType)
enum class EGCFDebugLogVerbosity : uint8
{
    Info,
    Success, // 緑色にする用
    Warning, // 黄色にする用
    Error    // グレーにする用（No Targetなど）
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

    // 更新したい項目（例: "Possession", "InputContext", "ReadyState"）
    UPROPERTY(BlueprintReadWrite)
    FString Label;

    // 表示する内容（例: "HumanPawn", "IMC_Vehicle"）
    UPROPERTY(BlueprintReadWrite)
    FString Value;

    // 状態による色指定（任意）
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