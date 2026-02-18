// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayEffectTypes.h"
#include "GCFGameplayEffectContext.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class AActor;
class FArchive;
class IGCFAbilitySourceInterface;
class UObject;
class UPhysicalMaterial;

USTRUCT()
struct FGCFGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	FGCFGameplayEffectContext()
		: FGameplayEffectContext()
	{
	}

	FGCFGameplayEffectContext(AActor* InInstigator, AActor* InEffectCauser)
		: FGameplayEffectContext(InInstigator, InEffectCauser)
	{
	}

	/** Returns the wrapped FGCFGameplayEffectContext from the handle, or nullptr if it doesn't exist or is the wrong type */
	static UE_API FGCFGameplayEffectContext* ExtractEffectContext(struct FGameplayEffectContextHandle Handle);

	/** Sets the object used as the ability source */
	void SetAbilitySource(const IGCFAbilitySourceInterface* InObject, float InSourceLevel);

	/** Returns the ability source interface associated with the source object. Only valid on the authority. */
	const IGCFAbilitySourceInterface* GetAbilitySource() const;

	virtual FGameplayEffectContext* Duplicate() const override
	{
		FGCFGameplayEffectContext* NewContext = new FGCFGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGCFGameplayEffectContext::StaticStruct();
	}

	///** Overridden to serialize new fields */
	//virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

	/** Returns the physical material from the hit result if there is one */
	const UPhysicalMaterial* GetPhysicalMaterial() const;

public:
	/** ID to allow the identification of multiple bullets that were part of the same cartridge */
	UPROPERTY()
	int32 CartridgeID = -1;

protected:
	/** Ability Source object (should implement IGCFAbilitySourceInterface). NOT replicated currently */
	UPROPERTY()
	TWeakObjectPtr<const UObject> AbilitySourceObject;
};

//template<>
//struct TStructOpsTypeTraits<FGCFGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FGCFGameplayEffectContext>
//{
//	enum
//	{
//		WithNetSerializer = true,
//		WithCopy = true
//	};
//};


#undef UE_API