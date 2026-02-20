// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "Movement/GCFMovementFunctionLibrary.h"

#include "Movement/GCFMovementConfigReceiver.h"
#include "Movement/GCFLocomotionHandler.h"


TScriptInterface<IGCFMovementConfigReceiver> UGCFMovementFunctionLibrary::ResolveMovementConfigReceiver(const UObject* Context)
{
	return GCF::Context::ResolveInterface<IGCFMovementConfigReceiver, UGCFMovementConfigReceiver>(Context);
}


TScriptInterface<IGCFLocomotionHandler> UGCFMovementFunctionLibrary::ResolveLocomotionHandler(const UObject* Context)
{
	return GCF::Context::ResolveInterface<IGCFLocomotionHandler, UGCFLocomotionHandler>(Context);
}
