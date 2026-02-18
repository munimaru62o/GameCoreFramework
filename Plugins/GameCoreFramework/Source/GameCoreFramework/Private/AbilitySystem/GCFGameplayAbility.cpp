// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GCFGameplayAbility.h"

#include "GCFShared.h"
#include "AbilitySystem/GCFAbilitySystemComponent.h"
//#include "AbilitySystemLog.h"
//#include "Player/GCFPlayerController.h"
//#include "Actor/Character/GCFCharacter.h"
//#include "Common/GCFGameplayTags.h"
//#include "GCFAbilityCost.h"
//#include "Character/GCFHeroComponent.h"
//#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "AbilitySystem/GCFAbilitySourceInterface.h"
#include "AbilitySystem/GCFGameplayEffectContext.h"
//#include "Physics/PhysicalMaterialWithTags.h"
//#include "GameFramework/PlayerState.h"
//#include "Camera/Mode/GCFCameraMode.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFGameplayAbility)

#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue)																				\
{																																						\
	if (!ensure(IsInstantiated()))																														\
	{																																					\
		ABILITY_LOG(Error, TEXT("%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName());	\
		return ReturnValue;																																\
	}																																					\
}

UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_SIMPLE_FAILURE_MESSAGE, "Ability.UserFacingSimpleActivateFail.Message");
UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_PLAY_MONTAGE_FAILURE_MESSAGE, "Ability.PlayMontageOnActivateFail.Message");

UGCFGameplayAbility::UGCFGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	ActivationPolicy = EGCFAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = EGCFAbilityActivationGroup::Independent;

	//bLogCancelation = false;
	//ActiveCameraMode = nullptr;
}

UGCFAbilitySystemComponent* UGCFGameplayAbility::GetGCFAbilitySystemComponentFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<UGCFAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr);
}

AController* UGCFGameplayAbility::GetControllerFromActorInfo() const
{
	return (CurrentActorInfo ? CurrentActorInfo->PlayerController.Get() : nullptr);
}

AActor* UGCFGameplayAbility::GetAvatarFromActorInfo() const
{
	return (CurrentActorInfo ? CurrentActorInfo->AvatarActor.Get() : nullptr);
}


void UGCFGameplayAbility::NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
{
	bool bSimpleFailureFound = false;
	for (FGameplayTag Reason : FailedReason)
	{
		if (!bSimpleFailureFound)
		{
			if (const FText* pUserFacingMessage = FailureTagToUserFacingMessages.Find(Reason))
			{
				FGCFAbilitySimpleFailureMessage Message;
				Message.PlayerController = GetActorInfo().PlayerController.Get();
				Message.FailureTags = FailedReason;
				Message.UserFacingReason = *pUserFacingMessage;

				UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
				MessageSystem.BroadcastMessage(TAG_ABILITY_SIMPLE_FAILURE_MESSAGE, Message);
				bSimpleFailureFound = true;
			}
		}
		
		if (UAnimMontage* pMontage = FailureTagToAnimMontage.FindRef(Reason))
		{
			FGCFAbilityMontageFailureMessage Message;
			Message.PlayerController = GetActorInfo().PlayerController.Get();
			Message.AvatarActor = GetActorInfo().AvatarActor.Get();
			Message.FailureTags = FailedReason;
			Message.FailureMontage = pMontage;

			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(TAG_ABILITY_PLAY_MONTAGE_FAILURE_MESSAGE, Message);
		}
	}
}

bool UGCFGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	//@TODO Possibly remove after setting up tag relationships
	UGCFAbilitySystemComponent* GCFASC = CastChecked<UGCFAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (GCFASC->IsActivationGroupBlocked(ActivationGroup))
	{
		if (OptionalRelevantTags)
		{
			OptionalRelevantTags->AddTag(GCFGameplayTags::Ability_ActivateFail_ActivationGroup);
		}
		return false;
	}

	return true;
}

void UGCFGameplayAbility::SetCanBeCanceled(bool bCanBeCanceled)
{
	// The ability can not block canceling if it's replaceable.
	if (!bCanBeCanceled && (ActivationGroup == EGCFAbilityActivationGroup::Exclusive_Replaceable))
	{
		UE_LOG(LogGCFAbilitySystem, Error, TEXT("SetCanBeCanceled: Ability [%s] can not block canceling because its activation group is replaceable."), *GetName());
		return;
	}

	Super::SetCanBeCanceled(bCanBeCanceled);
}

void UGCFGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	K2_OnAbilityAdded();

	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UGCFGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	K2_OnAbilityRemoved();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UGCFGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGCFGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	//ClearCameraMode();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

//bool UGCFGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
//{
//	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) || !ActorInfo)
//	{
//		return false;
//	}
//
//	// Verify we can afford any additional costs
//	for (const TObjectPtr<UGCFAbilityCost>& AdditionalCost : AdditionalCosts)
//	{
//		if (AdditionalCost != nullptr)
//		{
//			if (!AdditionalCost->CheckCost(this, Handle, ActorInfo, /*inout*/ OptionalRelevantTags))
//			{
//				return false;
//			}
//		}
//	}
//
//	return true;
//}

//void UGCFGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
//{
//	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
//
//	check(ActorInfo);
//
//	// Used to determine if the ability actually hit a target (as some costs are only spent on successful attempts)
//	auto DetermineIfAbilityHitTarget = [&]()
//	{
//		if (ActorInfo->IsNetAuthority())
//		{
//			if (UGCFAbilitySystemComponent* ASC = Cast<UGCFAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
//			{
//				FGameplayAbilityTargetDataHandle TargetData;
//				ASC->GetAbilityTargetData(Handle, ActivationInfo, TargetData);
//				for (int32 TargetDataIdx = 0; TargetDataIdx < TargetData.Data.Num(); ++TargetDataIdx)
//				{
//					if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetData, TargetDataIdx))
//					{
//						return true;
//					}
//				}
//			}
//		}
//
//		return false;
//	};
//
//	// Pay any additional costs
//	bool bAbilityHitTarget = false;
//	bool bHasDeterminedIfAbilityHitTarget = false;
//	for (const TObjectPtr<UGCFAbilityCost>& AdditionalCost : AdditionalCosts)
//	{
//		if (AdditionalCost != nullptr)
//		{
//			if (AdditionalCost->ShouldOnlyApplyCostOnHit())
//			{
//				if (!bHasDeterminedIfAbilityHitTarget)
//				{
//					bAbilityHitTarget = DetermineIfAbilityHitTarget();
//					bHasDeterminedIfAbilityHitTarget = true;
//				}
//
//				if (!bAbilityHitTarget)
//				{
//					continue;
//				}
//			}
//
//			AdditionalCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
//		}
//	}
//}
//
FGameplayEffectContextHandle UGCFGameplayAbility::MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	FGameplayEffectContextHandle ContextHandle = Super::MakeEffectContext(Handle, ActorInfo);

	FGCFGameplayEffectContext* EffectContext = FGCFGameplayEffectContext::ExtractEffectContext(ContextHandle);
	check(EffectContext);

	check(ActorInfo);

	AActor* EffectCauser = nullptr;
	const IGCFAbilitySourceInterface* AbilitySource = nullptr;
	float SourceLevel = 0.0f;
	GetAbilitySource(Handle, ActorInfo, /*out*/ SourceLevel, /*out*/ AbilitySource, /*out*/ EffectCauser);

	UObject* SourceObject = GetSourceObject(Handle, ActorInfo);

	AActor* Instigator = ActorInfo ? ActorInfo->OwnerActor.Get() : nullptr;

	EffectContext->SetAbilitySource(AbilitySource, SourceLevel);
	EffectContext->AddInstigator(Instigator, EffectCauser);
	EffectContext->AddSourceObject(SourceObject);

	return ContextHandle;
}
//
//void UGCFGameplayAbility::ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const
//{
//	Super::ApplyAbilityTagsToGameplayEffectSpec(Spec, AbilitySpec);
//
//	if (const FHitResult* HitResult = Spec.GetContext().GetHitResult())
//	{
//		if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(HitResult->PhysMaterial.Get()))
//		{
//			Spec.CapturedTargetTags.GetSpecTags().AppendTags(PhysMatWithTags->Tags);
//		}
//	}
//}
//
bool UGCFGameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	// Specialized version to handle death exclusion and AbilityTags expansion via ASC

	bool bBlocked = false;
	bool bMissing = false;

	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
	const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

	// Check if any of this ability's tags are currently blocked
	if (AbilitySystemComponent.AreAbilityTagsBlocked(GetAssetTags()))
	{
		bBlocked = true;
	}

	const UGCFAbilitySystemComponent* GCFASC = Cast<UGCFAbilitySystemComponent>(&AbilitySystemComponent);
	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;

	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;

	// Expand our ability tags to add additional required/blocked tags
	if (GCFASC)
	{
		GCFASC->GetAdditionalActivationTagRequirements(GetAssetTags(), AllRequiredTags, AllBlockedTags);
	}

	// Check to see the required/blocked tags for this ability
	if (AllBlockedTags.Num() || AllRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;
		
		AbilitySystemComponentTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (AbilitySystemComponentTags.HasAny(AllBlockedTags))
		{
			if (OptionalRelevantTags && AbilitySystemComponentTags.HasTag(GCFGameplayTags::Status_Death))
			{
				// If player is dead and was rejected due to blocking tags, give that feedback
				OptionalRelevantTags->AddTag(GCFGameplayTags::Ability_ActivateFail_IsDead);
			}

			bBlocked = true;
		}

		if (!AbilitySystemComponentTags.HasAll(AllRequiredTags))
		{
			bMissing = true;
		}
	}

	if (SourceTags != nullptr)
	{
		if (SourceBlockedTags.Num() || SourceRequiredTags.Num())
		{
			if (SourceTags->HasAny(SourceBlockedTags))
			{
				bBlocked = true;
			}

			if (!SourceTags->HasAll(SourceRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (TargetTags != nullptr)
	{
		if (TargetBlockedTags.Num() || TargetRequiredTags.Num())
		{
			if (TargetTags->HasAny(TargetBlockedTags))
			{
				bBlocked = true;
			}

			if (!TargetTags->HasAll(TargetRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (bBlocked)
	{
		if (OptionalRelevantTags && BlockedTag.IsValid())
		{
			OptionalRelevantTags->AddTag(BlockedTag);
		}
		return false;
	}
	if (bMissing)
	{
		if (OptionalRelevantTags && MissingTag.IsValid())
		{
			OptionalRelevantTags->AddTag(MissingTag);
		}
		return false;
	}

	return true;
}

void UGCFGameplayAbility::OnPawnAvatarSet()
{
	K2_OnPawnAvatarSet();
}

void UGCFGameplayAbility::GetAbilitySource(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& OutSourceLevel, const IGCFAbilitySourceInterface*& OutAbilitySource, AActor*& OutEffectCauser) const
{
	OutSourceLevel = 0.0f;
	OutAbilitySource = nullptr;
	OutEffectCauser = nullptr;

	OutEffectCauser = ActorInfo->AvatarActor.Get();

	// If we were added by something that's an ability info source, use it
	UObject* SourceObject = GetSourceObject(Handle, ActorInfo);

	OutAbilitySource = Cast<IGCFAbilitySourceInterface>(SourceObject);
}


void UGCFGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
{
	// Try to activate if activation policy is on spawn.
	if (ActorInfo && !Spec.IsActive() && (ActivationPolicy == EGCFAbilityActivationPolicy::OnSpawn))
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

		// If avatar actor is torn off or about to die, don't try to activate until we get the new one.
		if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
		{
			const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
			const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

			const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
			const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

			if (bClientShouldActivate || bServerShouldActivate)
			{
				ASC->TryActivateAbility(Spec.Handle);
			}
		}
	}
}

//bool UGCFGameplayAbility::CanChangeActivationGroup(EGCFAbilityActivationGroup NewGroup) const
//{
//	if (!IsInstantiated() || !IsActive())
//	{
//		return false;
//	}
//
//	if (ActivationGroup == NewGroup)
//	{
//		return true;
//	}
//
//	UGCFAbilitySystemComponent* GCFASC = GetGCFAbilitySystemComponentFromActorInfo();
//	check(GCFASC);
//
//	if ((ActivationGroup != EGCFAbilityActivationGroup::Exclusive_Blocking) && GCFASC->IsActivationGroupBlocked(NewGroup))
//	{
//		// This ability can't change groups if it's blocked (unless it is the one doing the blocking).
//		return false;
//	}
//
//	if ((NewGroup == EGCFAbilityActivationGroup::Exclusive_Replaceable) && !CanBeCanceled())
//	{
//		// This ability can't become replaceable if it can't be canceled.
//		return false;
//	}
//
//	return true;
//}
//
//bool UGCFGameplayAbility::ChangeActivationGroup(EGCFAbilityActivationGroup NewGroup)
//{
//	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(ChangeActivationGroup, false);
//
//	if (!CanChangeActivationGroup(NewGroup))
//	{
//		return false;
//	}
//
//	if (ActivationGroup != NewGroup)
//	{
//		UGCFAbilitySystemComponent* GCFASC = GetGCFAbilitySystemComponentFromActorInfo();
//		check(GCFASC);
//
//		GCFASC->RemoveAbilityFromActivationGroup(ActivationGroup, this);
//		GCFASC->AddAbilityToActivationGroup(NewGroup, this);
//
//		ActivationGroup = NewGroup;
//	}
//
//	return true;
//}
//
//void UGCFGameplayAbility::SetCameraMode(TSubclassOf<UGCFCameraMode> CameraMode)
//{
//	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(SetCameraMode, );
//
//	if (UGCFHeroComponent* HeroComponent = GetHeroComponentFromActorInfo())
//	{
//		HeroComponent->SetAbilityCameraMode(CameraMode, CurrentSpecHandle);
//		ActiveCameraMode = CameraMode;
//	}
//}
//
//void UGCFGameplayAbility::ClearCameraMode()
//{
//	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(ClearCameraMode, );
//
//	if (ActiveCameraMode)
//	{
//		if (UGCFHeroComponent* HeroComponent = GetHeroComponentFromActorInfo())
//		{
//			HeroComponent->ClearAbilityCameraMode(CurrentSpecHandle);
//		}
//
//		ActiveCameraMode = nullptr;
//	}
//}
//
