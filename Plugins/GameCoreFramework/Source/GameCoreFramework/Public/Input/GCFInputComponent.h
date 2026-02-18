// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 62o. All rights reserved.

#pragma once

#include "EnhancedInputComponent.h"

#include "Input/GCFInputTypes.h"
#include "GCFInputConfig.h"

#include "GCFInputComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UObject;
class UGCFInputConfig;
class UGCFInputComponent;


/**
 * @brief Extended EnhancedInputComponent that enables "Tag-based" input binding.
 *
 * [Problem Solved]
 * Standard Enhanced Input requires direct references to UInputAction assets in C++.
 * This creates tight coupling between code and assets.
 *
 * [Solution]
 * This component allows binding to "GameplayTags" (e.g., Input.Action.Jump) instead of assets.
 * The mapping from Tag to Asset is handled by the UGCFInputConfig data asset.
 * This allows designers to swap input assets without touching C++ code.
 */
UCLASS(Config = Input)
class UGCFInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:

	UGCFInputComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * Helper to add Input Mapping Contexts (IMC) defined in the config to the local player subsystem.
	 * Typically called when a Pawn is possessed or a weapon is equipped.
	 */
	void AddInputMappings(const UGCFInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	/** Removes the mapping contexts. */
	void RemoveInputMappings(const UGCFInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	// ----------------------------------------------------------------------------------------------------------------
	// Native Action Binding (Trigger Event -> Function Call)
	// Supports varied function signatures (Void, FInputActionValue, FInputActionInstance)
	// ----------------------------------------------------------------------------------------------------------------
	template<class UserClass, typename... VarTypes>
	FGCFBindingReceipt BindNativeAction(const UGCFInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, typename FEnhancedInputActionHandlerSignature::TMethodPtr<UserClass, VarTypes...> Func, VarTypes... Vars);
	template<class UserClass, typename... VarTypes>
	FGCFBindingReceipt BindNativeAction(const UGCFInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, typename FEnhancedInputActionHandlerValueSignature::TMethodPtr<UserClass, VarTypes...> Func, VarTypes... Vars);
	template<class UserClass, typename... VarTypes>
	FGCFBindingReceipt BindNativeAction(const UGCFInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, typename FEnhancedInputActionHandlerInstanceSignature::TMethodPtr<UserClass, VarTypes...> Func, VarTypes... Vars);

	// ----------------------------------------------------------------------------------------------------------------
	// Ability Action Binding (Pressed/Released -> Tag Routing)
	// ----------------------------------------------------------------------------------------------------------------

	/**
	 * Binds all actions defined in the "AbilityInputActions" list of the config.
	 * Instead of calling a specific function per action, it calls a single generic handler
	 * (PressedFunc/ReleasedFunc) passing the GameplayTag as an argument.
	 */
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	TArray<FGCFBindingReceipt> BindAbilityActions(const UGCFInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc);
};

/**
 * @brief Helper struct to simplify the syntax of binding multiple actions.
 * Automatically aggregates binding receipts into an array for easy lifecycle management.
 */
struct FGCFInputBinder
{
	UGCFInputComponent* IC;
	const UGCFInputConfig* Config;
	TArray<FGCFBindingReceipt>& Receipts;

	FGCFInputBinder(UGCFInputComponent* InIC, const UGCFInputConfig* InConfig, TArray<FGCFBindingReceipt>& InReceipts)
		: IC(InIC), Config(InConfig), Receipts(InReceipts)
	{}

	// Wrapper for BindNativeAction (Void signature)
	template<class UserClass, typename... VarTypes>
	void Bind(const FGameplayTag& Tag, ETriggerEvent Event, UserClass* Obj, typename FEnhancedInputActionHandlerSignature::TMethodPtr<UserClass, VarTypes...> Func, VarTypes... Vars)
	{
		if (IC && Config) {
			FGCFBindingReceipt Receipt = IC->BindNativeAction(Config, Tag, Event, Obj, Func, Vars...);
			if (Receipt.BindingPtr) {
				Receipts.Add(Receipt);
			}
		}
	}

	// Wrapper for BindNativeAction (Value signature)
	template<class UserClass, typename... VarTypes>
	void Bind(const FGameplayTag& Tag, ETriggerEvent Event, UserClass* Obj, typename FEnhancedInputActionHandlerValueSignature::TMethodPtr<UserClass, VarTypes...> Func, VarTypes... Vars)
	{
		if (IC && Config) {
			FGCFBindingReceipt Receipt = IC->BindNativeAction(Config, Tag, Event, Obj, Func, Vars...);
			if (Receipt.BindingPtr) {
				Receipts.Add(Receipt);
			}
		}
	}

	// Wrapper for BindNativeAction (Instance signature)
	template<class UserClass, typename... VarTypes>
	void Bind(const FGameplayTag& Tag, ETriggerEvent Event, UserClass* Obj, typename FEnhancedInputActionHandlerInstanceSignature::TMethodPtr<UserClass, VarTypes...> Func, VarTypes... Vars)
	{
		if (IC && Config) {
			FGCFBindingReceipt Receipt = IC->BindNativeAction(Config, Tag, Event, Obj, Func, Vars...);
			if (Receipt.BindingPtr) {
				Receipts.Add(Receipt);
			}
		}
	}

	// Wrapper for BindAbilityActions
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbility(UserClass* Obj, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc)
	{
		if (IC && Config) {
			Receipts.Append(IC->BindAbilityActions(Config, Obj, PressedFunc, ReleasedFunc));
		}
	}
};

// ----------------------------------------------------------------------------------------------------------------
// Template Implementations
// ----------------------------------------------------------------------------------------------------------------

// 1. Void Function Signature
template<class UserClass, typename... VarTypes>
FGCFBindingReceipt UGCFInputComponent::BindNativeAction(const UGCFInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, typename FEnhancedInputActionHandlerSignature::TMethodPtr<UserClass, VarTypes...> Func, VarTypes... Vars)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag)) {
		FEnhancedInputActionEventBinding* Binding = &BindAction(IA, TriggerEvent, Object, Func, Vars...);
		return FGCFBindingReceipt(Binding, InputTag, TriggerEvent);
	}
	return FGCFBindingReceipt();
}

// 2. FInputActionValue Signature
template<class UserClass, typename... VarTypes>
FGCFBindingReceipt UGCFInputComponent::BindNativeAction(const UGCFInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, typename FEnhancedInputActionHandlerValueSignature::TMethodPtr<UserClass, VarTypes...> Func, VarTypes... Vars)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag)) {
		FEnhancedInputActionEventBinding* Binding = &BindAction(IA, TriggerEvent, Object, Func, Vars...);
		return FGCFBindingReceipt(Binding, InputTag, TriggerEvent);
	}
	return FGCFBindingReceipt();
}

// 3. FInputActionInstance Signature
template<class UserClass, typename... VarTypes>
FGCFBindingReceipt UGCFInputComponent::BindNativeAction(const UGCFInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, typename FEnhancedInputActionHandlerInstanceSignature::TMethodPtr<UserClass, VarTypes...> Func, VarTypes... Vars)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag)) {
		FEnhancedInputActionEventBinding* Binding = &BindAction(IA, TriggerEvent, Object, Func, Vars...);
		return FGCFBindingReceipt(Binding, InputTag, TriggerEvent);
	}
	return FGCFBindingReceipt();
}


template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
TArray<FGCFBindingReceipt> UGCFInputComponent::BindAbilityActions(const UGCFInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc)
{
	check(InputConfig);

	TArray<FGCFBindingReceipt> OutReceipts;
	OutReceipts.Reserve(InputConfig->AbilityInputActions.Num() * 2);

	for (const FGCFInputAction& Action : InputConfig->AbilityInputActions) {
		if (Action.InputAction && Action.InputTag.IsValid()) {
			if (PressedFunc) {
				const ETriggerEvent TriggerEvent = ETriggerEvent::Triggered;
				FEnhancedInputActionEventBinding* Binding = &BindAction(Action.InputAction, TriggerEvent, Object, PressedFunc, Action.InputTag);
				OutReceipts.Add(FGCFBindingReceipt(Binding, Action.InputTag, TriggerEvent));
			}

			if (ReleasedFunc) {
				const ETriggerEvent TriggerEvent = ETriggerEvent::Completed;
				FEnhancedInputActionEventBinding* Binding = &BindAction(Action.InputAction, TriggerEvent, Object, ReleasedFunc, Action.InputTag);
				OutReceipts.Add(FGCFBindingReceipt(Binding, Action.InputTag, TriggerEvent));
			}
		}
	}
	return OutReceipts;
}