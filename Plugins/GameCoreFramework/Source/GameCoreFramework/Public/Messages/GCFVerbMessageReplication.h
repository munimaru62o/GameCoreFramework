// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "GCFVerbMessage.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "GCFVerbMessageReplication.generated.h"

class UObject;
struct FGCFVerbMessageReplication;
struct FNetDeltaSerializeInfo;

/**
 * Represents one verb message
 */
USTRUCT(BlueprintType)
struct FGCFVerbMessageReplicationEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FGCFVerbMessageReplicationEntry()
	{}

	FGCFVerbMessageReplicationEntry(const FGCFVerbMessage& InMessage)
		: Message(InMessage)
	{
	}

	FString GetDebugString() const;

private:
	friend FGCFVerbMessageReplication;

	UPROPERTY()
	FGCFVerbMessage Message;
};

/** Container of verb messages to replicate */
USTRUCT(BlueprintType)
struct FGCFVerbMessageReplication : public FFastArraySerializer
{
	GENERATED_BODY()

	FGCFVerbMessageReplication()
	{
	}

public:
	void SetOwner(UObject* InOwner) { Owner = InOwner; }

	// Broadcasts a message from server to clients
	void AddMessage(const FGCFVerbMessage& Message);

	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGCFVerbMessageReplicationEntry, FGCFVerbMessageReplication>(CurrentMessages, DeltaParms, *this);
	}

private:
	void RebroadcastMessage(const FGCFVerbMessage& Message);

private:
	// Replicated list of gameplay tag stacks
	UPROPERTY()
	TArray<FGCFVerbMessageReplicationEntry> CurrentMessages;
	
	// Owner (for a route to a world)
	UPROPERTY()
	TObjectPtr<UObject> Owner = nullptr;
};

template<>
struct TStructOpsTypeTraits<FGCFVerbMessageReplication> : public TStructOpsTypeTraitsBase2<FGCFVerbMessageReplication>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
