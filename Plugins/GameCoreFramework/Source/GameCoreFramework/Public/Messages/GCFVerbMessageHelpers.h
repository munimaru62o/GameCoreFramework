// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GCFVerbMessageHelpers.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

struct FGameplayCueParameters;
struct FGCFVerbMessage;

class APlayerController;
class APlayerState;
class UObject;
struct FFrame;


UCLASS(MinimalAPI)
class UGCFVerbMessageHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "GCF")
	static UE_API APlayerState* GetPlayerStateFromObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "GCF")
	static UE_API APlayerController* GetPlayerControllerFromObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "GCF")
	static UE_API FGameplayCueParameters VerbMessageToCueParameters(const FGCFVerbMessage& Message);

	UFUNCTION(BlueprintCallable, Category = "GCF")
	static UE_API FGCFVerbMessage CueParametersToVerbMessage(const FGameplayCueParameters& Params);
};

#undef UE_API
