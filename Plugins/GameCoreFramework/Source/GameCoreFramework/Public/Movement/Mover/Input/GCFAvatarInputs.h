// Copyright (c) 2026 munimaru62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "MoverTypes.h"
#include "GCFAvatarInputs.generated.h"

/**
 * @brief Structure containing extended movement and ability inputs specific to Avatar characters.
 * 
 * This struct encapsulates additional input intents (like crouching, sprinting) that go beyond
 * the standard directional movement, tailored specifically for bipedal characters.
 */
USTRUCT(BlueprintType)
struct GAMECOREFRAMEWORK_API FGCFAvatarInputs : public FMoverDataStructBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GCF|Mover")
	bool bWantsToCrouch = false;

	// TODO: Future inputs such as sprinting or parkour intents should be added here.
	// bool bWantsToSprint = false;

	// --- FMoverDataStructBase Implementation ---

	virtual bool ShouldReconcile(const FMoverDataStructBase& AuthorityState) const override
	{
		const FGCFAvatarInputs& TypedAuthority = static_cast<const FGCFAvatarInputs&>(AuthorityState);
		return (TypedAuthority.bWantsToCrouch != bWantsToCrouch);
	}

	virtual void Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float LerpFactor) override
	{
		// Since boolean values cannot be interpolated (Lerped), we adopt the value from the 
		// source state if LerpFactor is less than 0.5; otherwise, we use the target state.
		const FGCFAvatarInputs& SourceInputs = static_cast<const FGCFAvatarInputs&>((LerpFactor < 0.5f) ? From : To);
		bWantsToCrouch = SourceInputs.bWantsToCrouch;
	}

	virtual void Merge(const FMoverDataStructBase& From) override
	{
		const FGCFAvatarInputs& TypedFrom = static_cast<const FGCFAvatarInputs&>(From);
		bWantsToCrouch |= TypedFrom.bWantsToCrouch;
	}

	virtual FMoverDataStructBase* Clone() const override
	{
		return new FGCFAvatarInputs(*this);
	}

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override
	{
		Super::NetSerialize(Ar, Map, bOutSuccess);

		// Optimize network bandwidth by serializing boolean flags down to a single bit.
		Ar.SerializeBits(&bWantsToCrouch, 1);

		bOutSuccess = true;
		return true;
	}

	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }

	virtual void ToString(FAnsiStringBuilderBase& Out) const override
	{
		Super::ToString(Out);
		Out.Appendf("bWantsToCrouch: %i\n", bWantsToCrouch);
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override { Super::AddReferencedObjects(Collector); }
};