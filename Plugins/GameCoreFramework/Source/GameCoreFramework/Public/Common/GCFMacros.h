// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Input/GCFInputBindingManagerComponent.h"

/**
 * @brief Macro to simplify the registration of input bindings from components.
 *
 * This macro handles the creation of the delegate and automatically generates a unique
 * identifier key based on the class name and function name.
 *
 * [Benefit]
 * - Reduces boilerplate code.
 * - Prevents copy-paste errors where the KeyName might not match the actual function.
 *
 * [Usage]
 * GCF_REGISTER_INPUT_BINDING(this, &ThisClass::HandleInputBinding);
 *
 * @param Context       The object instance registering the binding (usually 'this').
 * @param CallbackFunc  The member function pointer to bind (e.g., &ThisClass::FunctionName).
 */
#define GCF_REGISTER_INPUT_BINDING(Context, CallbackFunc) \
	{ \
		FGCFInputBindNativeDelegate BindingDelegate; \
		BindingDelegate.BindUObject(Context, CallbackFunc); \
		/* Construct a unique and readable key: "ClassName::FunctionName" */ \
		const FString KeyString = FString::Printf(TEXT("%s::%s"), *Context->GetClass()->GetName(), TEXT(#CallbackFunc)); \
		UGCFInputBindingManagerComponent::RegisterInputBinding(Context, FName(*KeyString), MoveTemp(BindingDelegate)); \
	}