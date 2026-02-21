// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/Class.h"
#include "Common/GCFDebugTypes.h"
#include "System/Lifecycle/GCFStateTypes.h"
#include "GCFDebugFunctionLibrary.generated.h"

#define UE_API GAMECOREFRAMEWORK_API


/**
 * Utility library for formatting and broadcasting debug information.
 * Uses the Gameplay Message Subsystem to dispatch logs and states without direct UI dependencies.
 */
UCLASS(Abstract, MinimalAPI)
class UGCFDebugFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    /**
     * Extracts the string name of an enum value (excluding the namespace).
     */
	template<typename TEnum>
	static FString GetEnumName(TEnum EnumValue)
	{
		const UEnum* EnumPtr = StaticEnum<TEnum>();
		if (!EnumPtr) {
			return TEXT("InvalidEnum");
		}
		return EnumPtr->GetNameStringByValue((int64)EnumValue);
	}

    /**
      * Converts a bitmask enum into a pipe-separated string (e.g., "FlagA | FlagB").
      * Ignores zero, hidden, and automatically generated "_MAX" values.
      */
    template<typename TEnum>
	static FString GetBitflagsString(TEnum Bitmask)
	{
		const UEnum* EnumPtr = StaticEnum<TEnum>();
		if (!EnumPtr) return TEXT("Invalid");

		if ((int64)Bitmask == 0) return TEXT("None");

		TArray<FString> Results;

		for (int32 i = 0; i < EnumPtr->NumEnums(); i++) {
			int64 Val = EnumPtr->GetValueByIndex(i);

			if (Val == 0) continue;
#if WITH_EDITOR
			if (EnumPtr->HasMetaData(TEXT("Hidden"), i)) continue;
#endif
			FString EnumName = EnumPtr->GetNameStringByIndex(i);
			if (EnumName.Contains(TEXT("_MAX"))) continue;

			if (((int64)Bitmask & Val) == Val) {
				Results.Add(GetEnumName((TEnum)Val));
			}
		}
		return Results.Num() > 0 ? FString::Join(Results, TEXT(" | ")) : TEXT("Unknown");
	}


	/**
	 * Formats a log message with RichText tags based on its verbosity.
	 */
    UFUNCTION(BlueprintPure, Category = "GCF|Debug", meta = (Keywords = "Log Format RichText"))
    static FText FormatLogMessage(EGCFDebugLogVerbosity Verbosity, const FString& InMessage);

	/** Broadcasts a general debug log message via the Gameplay Message Subsystem. */
	UFUNCTION(BlueprintCallable, Category = "GCF|Debug")
	static void SendLogMessage(const UObject* WorldContext, EGCFDebugLogVerbosity Verbosity, const FString& Msg);

	/** Broadcasts a string-based state change for debug visualization. */
	UFUNCTION(BlueprintCallable, Category = "GCF|Debug")
	static void SendStateMessage(const UObject* WorldContext, EGCFDebugStateCategory Category, const FString& NewValue, const FLinearColor& DisplayColor = FLinearColor::White);

	/** Broadcasts a bitmask state change (Player Ready State) for debug visualization. */
	UFUNCTION(BlueprintCallable, Category = "GCF|Debug")
	static void SendPlayerStateBitMessage(const UObject* WorldContext, EGCFDebugStateCategory Category, EGCFPlayerReadyState State, const FLinearColor& DisplayColor = FLinearColor::White);

	/** Broadcasts a bitmask state change (Pawn Ready State) for debug visualization. */
	UFUNCTION(BlueprintCallable, Category = "GCF|Debug")
	static void SendPawnStateBitMessage(const UObject* WorldContext, EGCFDebugStateCategory Category, EGCFPawnReadyState State, const FLinearColor& DisplayColor = FLinearColor::White);
};

#undef UE_API