// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 62o. All rights reserved.

#include "Actor/Data/GCFPawnData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFPawnData)

UGCFPawnData::UGCFPawnData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PawnClass = nullptr;
	TagRelationshipMapping = nullptr;
	InputConfig = nullptr;
	DefaultCameraMode = nullptr;
}

