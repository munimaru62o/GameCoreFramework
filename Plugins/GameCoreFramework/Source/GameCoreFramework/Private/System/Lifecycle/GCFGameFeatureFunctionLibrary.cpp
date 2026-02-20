// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.


#include "System/Lifecycle/GCFGameFeatureFunctionLibrary.h"
#include "GCFShared.h"
#include "Components/GameFrameworkComponentManager.h"


void UGCFGameFeatureFunctionLibrary::ReadyFeature(UObject* Implementer, AActor* Actor, FName FeatureName)
{
	if (UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(Actor)) {
		ReadyFeature(GFCM, Implementer, Actor, FeatureName);
	}
}

void UGCFGameFeatureFunctionLibrary::ReadyFeature(UGameFrameworkComponentManager* GFCM, UObject* Implementer, AActor* Actor, FName FeatureName)
{
	if (GFCM) {
		GFCM->ChangeFeatureInitState(Actor, FeatureName, Implementer, GCFGameplayTags::FeatureState_Ready);
	}
}


void UGCFGameFeatureFunctionLibrary::InitFeature(UObject* Implementer, AActor* Actor, FName FeatureName)
{
	if (UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(Actor)) {
		InitFeature(GFCM, Implementer, Actor, FeatureName);
	}
}

void UGCFGameFeatureFunctionLibrary::InitFeature(UGameFrameworkComponentManager* GFCM, UObject* Implementer, AActor* Actor, FName FeatureName)
{
	if (GFCM) {
		GFCM->ChangeFeatureInitState(Actor, FeatureName, Implementer, GCFGameplayTags::FeatureState_Initial);
	}
}


bool UGCFGameFeatureFunctionLibrary::IsFeatureReady(AActor* Actor, FName FeatureName)
{
	if (UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(Actor)) {
		return IsFeatureReady(GFCM, Actor, FeatureName);
	}
	return false;
}

bool UGCFGameFeatureFunctionLibrary::IsFeatureReady(UGameFrameworkComponentManager* GFCM, AActor* Actor, FName FeatureName)
{
	if (GFCM) {
		return IsFeatureTag(GFCM, Actor, FeatureName, GCFGameplayTags::FeatureState_Ready);
	}
	return false;
}


bool UGCFGameFeatureFunctionLibrary::IsFeatureTag(AActor* Actor, FName FeatureName, FGameplayTag Tag)
{
	if (UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(Actor)) {
		return IsFeatureTag(GFCM, Actor, FeatureName, Tag);
	}
	return false;
}

bool UGCFGameFeatureFunctionLibrary::IsFeatureTag(UGameFrameworkComponentManager* GFCM, AActor* Actor, FName FeatureName, FGameplayTag Tag)
{
	if (GFCM) {
		const FGameplayTag CurrentTag = GFCM->GetInitStateForFeature(Actor, FeatureName);
		if (CurrentTag == Tag) {
			return true;
		}
	}
	return false;
}