// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/GCFGameData.h"
#include "System/Asset/GCFAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFGameData)

UGCFGameData::UGCFGameData()
{
}

const UGCFGameData& UGCFGameData::UGCFGameData::Get()
{
	return UGCFAssetManager::Get().GetGameData();
}
