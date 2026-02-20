// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#pragma once

#include "Logging/LogMacros.h"

#define UE_API GAMECOREFRAMEWORK_API

class UObject;

// UE_LOG with Location
#define UE_LOG_LOC(Category, Verbosity, Format, ...) UE_LOG(Category, Verbosity, TEXT("%s (%s:%d): ") Format, ANSI_TO_TCHAR(__FUNCTION__), ANSI_TO_TCHAR(__FILE__), __LINE__, ##__VA_ARGS__)

UE_API DECLARE_LOG_CATEGORY_EXTERN(LogGCFCommon, Log, All);
UE_API DECLARE_LOG_CATEGORY_EXTERN(LogGCFAbilitySystem, Log, All);
UE_API DECLARE_LOG_CATEGORY_EXTERN(LogGCFCharacter, Log, All);
UE_API DECLARE_LOG_CATEGORY_EXTERN(LogGCFPlayer, Log, All);
UE_API DECLARE_LOG_CATEGORY_EXTERN(LogGCFSystem, Log, All);
UE_API DECLARE_LOG_CATEGORY_EXTERN(LogGCFUI, Log, All);
UE_API DECLARE_LOG_CATEGORY_EXTERN(LogGCFModular, Log, All);

UE_API FString GetClientServerContextString(UObject* ContextObject = nullptr);

#undef UE_API