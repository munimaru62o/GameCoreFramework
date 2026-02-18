// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputTriggers.h"
#include "GCFInputTypes.generated.h"


struct FEnhancedInputActionEventBinding;

/**
 * @brief Represents a handle (receipt) for a registered input binding.
 *
 * This struct acts as a bridge between the raw Enhanced Input binding pointer
 * and the high-level GameplayTag system.
 *
 * [Use Case]
 * - Used to track which GameplayTag is bound to which internal action.
 * - Essential for debugging (knowing "Why is 'Ability.Jump' not firing?").
 * - Used for safe unbinding (removal) of specific actions.
 */
USTRUCT(BlueprintType)
struct FGCFBindingReceipt
{
	GENERATED_BODY()

	/** Raw pointer to the binding created by the Enhanced Input subsystem. */
	FEnhancedInputActionEventBinding* BindingPtr = nullptr;

	/** The GameplayTag associated with this binding (e.g., "Input.Action.Jump"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FGameplayTag AssociatedTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	ETriggerEvent TriggerEvent = ETriggerEvent::None;

	FGCFBindingReceipt() = default;
	FGCFBindingReceipt(FEnhancedInputActionEventBinding* InPtr, const FGameplayTag& InTag, ETriggerEvent InEvent)
		: BindingPtr(InPtr), AssociatedTag(InTag), TriggerEvent(InEvent)
	{}
};


/**
 * @brief Bitmask flags representing the prerequisites for enabling Input.
 *
 * Used by the UGCFInputContextComponent (Gatekeeper) to manage the state of input availability.
 * Input is only allowed when specific combinations of these flags are set (e.g., PlayerReady | PawnReady).
 */
UENUM(BlueprintType)
enum class EGCFInputContextState : uint8
{
	None			= 0,
	PlayerReady		= 1 << 0,
	PawnReady		= 1 << 1,
};

ENUM_CLASS_FLAGS(EGCFInputContextState);

/**
 * @brief Category of the input device currently being used.
 * Useful for switching UI prompts (e.g., "Press A" vs "Press Space").
 */
UENUM(BlueprintType)
enum class EGCFInputDeviceType : uint8
{
	None = 0 UMETA(Hidden),
	Mouse,
	GamePad,
	Keyboard,
};

/**
 * @brief Categorizes the source of an input definition.
 * Used to determine lifecycle management (e.g., does this input persist across pawn death?).
 */
UENUM(BlueprintType)
enum class EGCFInputSourceType : uint8
{
	Invalid = 0 UMETA(Hidden),
	Player, // Persistent input (e.g., UI, Menus). Managed by Controller.
	Pawn,   // Avatar-specific input (e.g., Move, Attack). Managed by Pawn.
};

/** * Delegate broadcast when the input context state changes.
 * @param CurrentState The new bitmask state.
 * @param bInputEnabled Whether the aggregate state allows input processing.
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInputContextEvaluatedNative, EGCFInputContextState /* CurrentState */, bool /* bInputEnabled */);

/** Delegate broadcast when the InputComponent is ready/changed. */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInputComponentReady, UInputComponent*);