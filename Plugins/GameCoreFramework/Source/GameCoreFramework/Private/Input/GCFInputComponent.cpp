// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#include "Input/GCFInputComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Player/GCFLocalPlayer.h"
#include "Player/GCFSettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFInputComponent)

class UGCFInputConfig;

UGCFInputComponent::UGCFInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UGCFInputComponent::AddInputMappings(const UGCFInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Add Default Mapping Contexts
	// Note: We use a loop in case the config holds multiple contexts (e.g. Base + Weapon)
	// GCF/CommonGame often splits this, but here we assume direct addition.
	// (If InputConfig has a TArray<TPair<UInputMappingContext*, int32>> DefaultContexts)
	// Example implementation assuming the config structure:
	/*
	for (const auto& ContextMapping : InputConfig->DefaultMappingContexts)
	{
		if (ContextMapping.InputMappingContext)
		{
			InputSubsystem->AddMappingContext(ContextMapping.InputMappingContext, ContextMapping.Priority);
		}
	}
	*/
}

void UGCFInputComponent::RemoveInputMappings(const UGCFInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Example implementation:
	/*
	for (const auto& ContextMapping : InputConfig->DefaultMappingContexts)
	{
		if (ContextMapping.InputMappingContext)
		{
			InputSubsystem->RemoveMappingContext(ContextMapping.InputMappingContext);
		}
	}
	*/
}