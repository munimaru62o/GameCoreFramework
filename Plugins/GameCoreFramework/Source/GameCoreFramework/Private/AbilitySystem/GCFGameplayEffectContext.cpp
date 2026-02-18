// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GCFGameplayEffectContext.h"
#include "AbilitySystem/GCFAbilitySourceInterface.h"
#include "Engine/HitResult.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

//#if UE_WITH_IRIS
//#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
//#include "Serialization/GameplayEffectContextNetSerializer.h"
//#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFGameplayEffectContext)

class FArchive;

FGCFGameplayEffectContext* FGCFGameplayEffectContext::ExtractEffectContext(struct FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* BaseEffectContext = Handle.Get();
	if ((BaseEffectContext != nullptr) && BaseEffectContext->GetScriptStruct()->IsChildOf(FGCFGameplayEffectContext::StaticStruct()))
	{
		return (FGCFGameplayEffectContext*)BaseEffectContext;
	}

	return nullptr;
}

//bool FGCFGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
//{
//	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);
//
//	// Not serialized for post-activation use:
//	// CartridgeID
//
//	return true;
//}

//#if UE_WITH_IRIS
//namespace UE::Net
//{
//	// Forward to FGameplayEffectContextNetSerializer
//	// Note: If FGCFGameplayEffectContext::NetSerialize() is modified, a custom NetSerializesr must be implemented as the current fallback will no longer be sufficient.
//	UE_NET_IMPLEMENT_FORWARDING_NETSERIALIZER_AND_REGISTRY_DELEGATES(GCFGameplayEffectContext, FGameplayEffectContextNetSerializer);
//}
//#endif

void FGCFGameplayEffectContext::SetAbilitySource(const IGCFAbilitySourceInterface* InObject, float InSourceLevel)
{
	AbilitySourceObject = MakeWeakObjectPtr(Cast<const UObject>(InObject));
	//SourceLevel = InSourceLevel;
}

const IGCFAbilitySourceInterface* FGCFGameplayEffectContext::GetAbilitySource() const
{
	return Cast<IGCFAbilitySourceInterface>(AbilitySourceObject.Get());
}

const UPhysicalMaterial* FGCFGameplayEffectContext::GetPhysicalMaterial() const
{
	if (const FHitResult* HitResultPtr = GetHitResult())
	{
		return HitResultPtr->PhysMaterial.Get();
	}
	return nullptr;
}

