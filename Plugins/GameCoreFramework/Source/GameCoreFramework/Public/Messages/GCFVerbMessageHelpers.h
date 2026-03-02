// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GCFVerbMessageHelpers.generated.h"

struct FGameplayCueParameters;
struct FGCFVerbMessage;

class APlayerController;
class APlayerState;
class UObject;
struct FFrame;


UCLASS()
class GAMECOREFRAMEWORK_API UGCFVerbMessageHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "GCF")
	static APlayerState* GetPlayerStateFromObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "GCF")
	static APlayerController* GetPlayerControllerFromObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "GCF")
	static FGameplayCueParameters VerbMessageToCueParameters(const FGCFVerbMessage& Message);

	UFUNCTION(BlueprintCallable, Category = "GCF")
	static FGCFVerbMessage CueParametersToVerbMessage(const FGameplayCueParameters& Params);
};
